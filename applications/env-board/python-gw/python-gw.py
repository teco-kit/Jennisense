import serial
import urllib2
import urllib
import json
import serial
import string
from struct import unpack_from
from array import array


##### global server URL, if you change URL make sure you leave the %s in it
server_url="http://cumulus.teco.edu:51525/sensor/entity/%s" 


##################### hdlc

RF_END=0xC0
RF_ESC=0xDB
RF_T_END=0xDC
RF_T_ESC=0xDD

def checksum(ddata):
    """ calculate checksum on received hdlc data """
    ssum=0
    if(len(ddata)<3):
        return False
    for i in range(0,len(ddata)-2):
        ssum+=ddata[i]

    # from c code
    # in c ~ is simply not i.e. 1100 gets 0011
    # crc[0]=(~sum +1) & 0xff;
    # crc[1]=((~sum +1) >>8) & 0xff;
    # seems its the same in python ~ 

    crc= ( ((~ssum+1)&0xff) |  ( ( ( (~ssum +1) >>8) & 0xff)<<8) )
    
    value=ddata[len(ddata)-2];
    value=value|(ddata[len(ddata)-1]<<8);
    
    #print "checksums:"
    #print "recvd checksum: %s, calc'd checksum: %s"%(hex(value), hex(crc))
    #print "are same? %i"%( value==crc )
    
    return crc==value


def decode(data):
    """ decode data as hdlc packet """
    """ @return decoded data, is 0 if packet not valid """

    # array of type byte
    ddata=array('B')

    if data[0]==RF_END and data[len(data)-1]==RF_END:
        i=1
        while True:
            if data[i]==RF_END:
                break
            elif i<(len(data)-2):
                if (data[i]==RF_ESC) and (data[i+1]==RF_T_END):
                    ddata.append(RF_END)
                    i=i+1
                elif (data[i]==RF_ESC) and (data[i+1]==RF_T_ESC):
                    ddata.append(RF_ESC)
                    i=i+1
                else:
                    ddata.append(data[i])
            else:
                ddata.append(data[i]);
            i=i+1
        
    if(len(ddata)>0):
        if checksum(ddata)==True:
            return ddata

    return []


def receive_hdlc_packet(s):
    """ checks for beginning of packet on serial interface s """
    """ receives packet into buffer until end of packet and returns packet buffer """

    # receive buffer is a byte array
    data=array('B')
    try:
        while True:
            c = s.read()
            #print c,
            # beginning of packet?
            if (ord(c)==RF_END) and (len(data)==0):
                data.append(RF_END)
            # end of packet ?
            elif (ord(c)==RF_END) and (len(data)!=0):
                data.append(RF_END)
                return data 
            else: 
                data.append(ord(c))
    except:
        print "Exception while in receive packet. Ignoring. Proceeding."
        pass

##################### hdlc end


#################### concom

def con_com(a, b):
  tmp = ""
  d = ((a << 8) | b)
  c = (d % 40) + 0x33
  tmp += chr(c)
  d = d - c
  c = ((d % 1600) / 40) + 0x35
  tmp += chr(c)
  d = d - (c * 40);
  c = (d / 1600) + 0x35;
  tmp += chr(c)
  return tmp

################### concom




# format concom:  type_id: ["python parse string","concat character", "meaning of abbreviation string", "unit string"]
# if format string has more than a single element, results are fused together using str(X1)+concat char+str(X2)+concat char+..+str(N)
new_type_id_value_dict = {
            "ADR":[">16B",":", "JennicAddress", "MAC"],
            "AJN":[">B","" ,"Firmware Version", "Number"], 
            "CTS":[">Q","", "Timestamp", "ms"],
            "SAX":[">h","", "AccelXAxis", "g/s"],
            "SAY":[">h","", "AccelYAxis", "g/s"],
            "SAZ":[">h","", "AccelZAxis", "g/s"],
            "SGX":[">h","", "GyroXAxis", "deg/s"],
            "SGY":[">h","", "GyroYAxis", "deg/s"],
            "SGZ":[">h","", "GyroZAxis", "deg/s"],
            "SCX":[">h","", "HallXAxis", "h"],
            "SCY":[">h","", "HallYAxis", "h"],
            "SCZ":[">h","", "HallZAxis", "h"],
            "SHU":[">h","", "SHT21Humidity", "%"],
            "STE":[">h","", "SHT21Temperature", "mdegC"],
            "SXT":[">h","", "ExternalTemperature", "mdegC"],
            "STA":[">h","", "ExternalTemperature1", "mdegC"],
            "STB":[">h","", "ExternalTemperature2", "mdegC"]
        }


def map_data_dict_to_ubox_json_string(data_dict):
    """ this function converts the contents of data_dict to a json parsable string """
    """ the list structure should be as follows: type_id, value, value, value      """
    """ the type_id is searched for in the index list (defined above) and is replaced in the output by description """
    """ same goes for the value's units """
    """ if a type_id is not in the list then "unit" is "count" and the type_id is used as description for the data type """
    """ @param data_list list of the above described format """
    """ output string of structure: "ADJNs meaning":{"unit":"AJNs_unit_type", "value": "value_from_list"}, """
    """ we assume that ADR is contained in dict """

    json_struct ='{\n\
                     "data": \n\
                     { \
                        %s\n\
                     }\n\
                   }'

    # no description for this data present
    if "ADR" not in data_dict:
        print "Error there is no address in this data dict"
        return
    
    if len(data_dict)==0:
        print "Error there is nothing in this packet"
        return
    
    # prep json
    json_item='"%s":{"unit":"%s", "value":"%s"},'

    # collection of items
    json_collection=""

    # TODO: currently we can only handle single value elements
    # walk through dict and check if we have an entry in type_id_value_dict
    for data_item in data_dict:
        # is this item present in type id value -> map data to type
        if data_item in new_type_id_value_dict:
           this_item=json_item%(
                   new_type_id_value_dict[data_item][2],
                   new_type_id_value_dict[data_item][3],
                   data_dict[data_item]
                   )
        else: # if we have no info on this type, fall back to defaults
           this_item=json_item%(
                   data_item,
                   "counts",
                   data_dict[data_item]
                   )

        # add values
        json_collection+="\n"+this_item

    # remove last item's comma
    json_collection=json_collection[:-1]

    # put everything together and return
    json_struct=json_struct%json_collection
    return json_struct
   

def post_to_ubox(address, status):
    #url="http://localhost:5000"
    #url="http://129.13.169.222:51525/sensor/entity/66.1.1.1.1.1.1.66"
    global server_url
    # love python for this :)
    url=server_url%address
    try:
        #headers = {'X_REQUESTED_WITH' :'XMLHttpRequest','ACCEPT': 'application/json, text/javascript, */*; q=0.01',}
        data = json.loads(status)
        #data = json.loads('{"test": [{ "type":"address"}]}')
        #data = json.dumps({"data":{"move":{"unit":"e","value":"%s"}}}) % status
        #print "events2: %s" % status
        print data
        data=json.dumps(data)
        req = urllib2.Request(url, data, {'Content-Type': 'application/x-www-form-urlencoded' })
        req.get_method = lambda: 'PUT'
        f = urllib2.urlopen(req)
        response = f.read()
        f.close()
    except:
        print "Exception encountered while in post_to_ubox. Ignoring. Proceeding."
        pass


def process_data_based_on_type(packet):
    """ this function translates a packet to a dict of """
    """ dict[identifier]=value where values is based on format string given in mapping table """
    """ remarks: duplicates are replaced, unknown values are unpacked as hex byte strings """
    data_dict={}
    n=0
    while ( n < len(packet)-3 ):
        # get id of data in packet
        dtype= str(con_com(packet[n], packet[n+1]))
        
        # get len of data in packet
        data_len = packet[n+2]
        
        #print "POS: %i, found: %s, len: %i"%(n, dtype, data_len)
        #print "Packet Length:", len(packet)
        #print packet[n+3:n+3+data_len]

        # check if data type is known, then parse accordingly
        if (dtype in new_type_id_value_dict):
            # unpack based on fmtstring in new type id value dict
            res=unpack_from( new_type_id_value_dict[dtype][0], packet, n+3)

            # convert to string
            strres=str(res[0])


            # if there are more than 1 tuples returned 
            # add the remaining using the separator given in new_type_id_value_dict
            for i in range(1,len(res)):
                strres=strres + new_type_id_value_dict[dtype][1] + str(res[i])
            
            #print "unpack res for %s ==== %s "%(dtype, strres)
            data_dict[dtype]=strres
        
        # if data not known process as byte stream str and set as dtype data type in dict
        else:
            # print "unk %s "%(dtype)
            # check that length which was read from unk val is not bigger than packet
            if( (data_len-1 + n + 3) < len(packet) ):
                strres=""
                for i in range(0, data_len):
                    strres= strres+"{0:x}".format(packet[n+3+i])
                # indicate string as hex string    
                strres="0x"+strres
                data_dict[dtype]=strres

        n=n+3+data_len

    return data_dict


def main_json_to_ubox_data_hdlc():
    ser = serial.Serial('/dev/ttyACM0', 1000000, parity=serial.PARITY_NONE, stopbits=1, bytesize=8, rtscts=0, dsrdtr=0)
    adr="66.1.1.1.1.1.1.66"
    # flush everything buffer
    ser.flushInput()
    ser.flushOutput()

    while 1:
        
        packet=receive_hdlc_packet(ser)
        decoded_packet=decode(packet)
      
        # packet checksum was okay
        if(decoded_packet!=[]):
            # parse packet using concom
            #print decoded_packet
            data_dict=process_data_based_on_type(decoded_packet)

            # get adress of packet
            # default address if non could be found in sensor data
            if "ADR" not in data_dict:
                adr="66.1.1.1.1.1.1.66"
            else:                    
                adr=data_dict["ADR"]

            packet_json=map_data_dict_to_ubox_json_string(data_dict)
            print packet_json
            post_to_ubox(adr, packet_json)

        # ignore packet if crc failed
        else:
            continue




 
if __name__ == '__main__':
  #main()
  main_json_to_ubox_data_hdlc()
