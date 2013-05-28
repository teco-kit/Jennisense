#!/usr/bin/env python
# python script to check on jenv output for debugging possible hardware issues
# requires python packages:
#  * python-twisted  ....for async and simple serial
 
from twisted.protocols.basic import LineReceiver
from twisted.internet import reactor
from twisted.internet.serialport import SerialPort
from twisted.python import usage

import curses.wrapper
import logging
import sys
import time
import os
import datetime

#class fixedSerialPort(SerialPort):
#    def connectionLost(self, reason):
#        SerialPort.connectionLost(self, reason)
#        self.protocol.connectionLost(reason)



class THOptions(usage.Options):
    optParameters = [
        ['baudrate', 'b', 1e6, 'Serial baudrate'],
        ['port', 'p', '/dev/ttyACM0', 'Serial port to use'],
        ['fname', 'f',None, 'Filename to log data to']
        ]
  
  
class Echo(LineReceiver):
    # sensor array, the 0 is a placeholder an replaced by the line number when
    # the text is printed on the screen
    sensors={
            "acc":["Acceleration (LSM303DLM)" , 0 ],
            "mag":["Magnetometer (LSM303DLM)", 0], 
            "gyr":["Gyroscope (L3G4200D)", 0],
            "mcp_temp":["MCP Temperature (MPL115A2)",0],
            "mcp_pres":["MCP Pressure (MPL115A2)",0],
            "light":["Lightlevel (VCNL4000)",0],
            "prox":["Proximity (VCNL4000)",0],
            "sht_hum":["SHT21 Humidity",0],
            "sht_temp":["SHT21 Temperature",0],
            "ext_temp_i2c":["PT100 Temperature (ANALOGUE)",0]
#currently deactivated            "int_temp":["Internal Temperature",0]
        }

    # layout variables
    screen=0
    sensor_col=1
    status_col=50
    data_col=70
    fobj=None
    
    # maximal number of characters parsed
    max_len=30

    # must be overriden else it will check for \n\r which is not common
    # on linux systems
    delimiter='\n'
    def connectionMade(self):
        self.screen.addstr(4,self.sensor_col,'COMMUNICATION WITH JENV')
        self.screen.addstr(4,self.status_col,'OK                    ', curses.A_BOLD)

    def checkData(self, sensor, data):
        if len(data)>self.max_len:
            return False
        if "\n" in data:
            n=data.index("\n")
        else:
            n=len(data)
        return data[len(str(sensor))+1:n].strip()
        

    def parseData(self, line):
        """ identifies which kind of data was just received and sets status accordingly """
        for s in self.sensors:
            if s in line:

                # check if line is okay and extract data
                data=self.checkData(s,line)
                if data==False:
                    return

                # rewrite sensor name in none reversed mode
                self.screen.addstr(self.sensors[s][1],self.sensor_col,self.sensors[s][0])
                # remove line previously written at that position
                self.screen.addstr(self.sensors[s][1],self.data_col, " "*self.max_len)
                self.screen.addstr(self.sensors[s][1],self.data_col,data)

                # set column to okay for this sensor
                self.screen.addstr(self.sensors[s][1],self.status_col," "*10, curses.A_BOLD)
                self.screen.addstr(self.sensors[s][1],self.status_col,"OK", curses.A_BOLD)
            
                self.fobj.write('%s; %s; %s \n' %(time.time(),s, data))
        
        self.screen.refresh()
 
    def lineReceived(self, line):
        try:
            # TODO: test if this is a single line
            self.parseData(line)
        except ValueError:
            self.fobj.write('%s; ERROR: Unable to parse data; %s \n' %(time.time(),line))
            return   

    def setScreen(self, screen):
        self.screen=screen

    def initScreen(self):
        """ this function initializes the screen output """
        self.screen.addstr(0,0,"++++++++++++++ JENV TEST CODE +++++++++++++++", curses.A_BOLD)

        self.screen.addstr(2,self.sensor_col,'TYPE', curses.A_BOLD|curses.A_UNDERLINE)
        self.screen.addstr(2,self.status_col,'STATUS', curses.A_BOLD|curses.A_UNDERLINE)
        self.screen.addstr(2,self.data_col,'LAST DATA RCV', curses.A_BOLD|curses.A_UNDERLINE)

        self.screen.addstr(3,self.sensor_col,'SERIAL DEVICE /dev/ttyACM0', curses.A_REVERSE)
        self.screen.addstr(3,self.status_col,'NOT PRESENT', curses.A_BOLD|curses.A_REVERSE)
        self.screen.addstr(3,self.data_col,'PLEASE PLUGIN JENV BOARD AND SWITCH IT ON', curses.A_BOLD|curses.A_REVERSE)
        
        self.screen.addstr(4,self.sensor_col,'COMMUNICATION WITH JENV', curses.A_REVERSE)
        self.screen.addstr(4,self.status_col,'NOT ESTABLISHED', curses.A_BOLD|curses.A_REVERSE)
        i=5
        for s in self.sensors:
            self.screen.addstr(i,self.sensor_col,self.sensors[s][0], curses.A_REVERSE)
            self.sensors[s][1]=i
            self.screen.addstr(i,self.status_col, "NO DATA", curses.A_BOLD|curses.A_REVERSE)
            i=i+1

        self.screen.addstr(i+3,0, "CTRL+c to quit.")

        self.screen.refresh()
        
    def connectionLost(self, reason):
        self.screen.addstr(3,self.sensor_col,'SERIAL DEVICE /dev/ttyACM0')
        self.screen.addstr(3,self.status_col,'NOT PRESENT', curses.A_BOLD|curses.A_REVERSE)
        self.screen.addstr(3,self.data_col,'PLEASE PLUGIN JENV BOARD AND SWITCH IT ON', curses.A_BOLD|curses.A_REVERSE)
    
    


def initall(self):
    global baudrate, port, fname
    
    e=Echo()
    curses.use_default_colors()
    screen = curses.initscr()
    curses.noecho()
    e.setScreen(screen)
    
    e.initScreen()
    # must be raw to received sucessfully
    #e.setRawMode()
    port = o.opts['port']
  
    # todo print error when port not found
    # check if port exists
    while os.path.exists(port)==False:
        time.sleep(3)

    screen.addstr(3,e.sensor_col,'SERIAL DEVICE /dev/ttyACM0')
    screen.addstr(3,e.status_col,'OK         ', curses.A_BOLD)
    screen.addstr(3,e.data_col,len('PLEASE PLUGIN THE JENV BOARD AND SWITCH IT ON')*" ")
    screen.refresh()

    # todo set time out if nothing ever happens 

    s = SerialPort(e, port, reactor, baudrate=1e6)

    # open file write header, create file object
    e.fobj=open(fname,"w")
    # get current date and time
    st = datetime.datetime.fromtimestamp(time.time()).strftime('%Y-%m-%d %H:%M:%S')
    e.fobj.write("JENV LOG\nfile: %s\ntime: %s\n"%(fname, st))
      
    # todo flush port
    reactor.run()
    

  
if __name__ == '__main__':

    
    o = THOptions()
    
    try:
        o.parseOptions()
    except usage.UsageError, errortext:
        print '%s %s' % (sys.argv[0], errortext)
        print 'Try %s --help for usage details' % sys.argv[0]
        #logging.error('%s %s' % (sys.argv[0], errortext))
        #logging.info('Try %s --help for usage details' % sys.argv[0])
        raise SystemExit, 1
  
    if o.opts['baudrate']:
        baudrate = int(o.opts['baudrate'])
    
    if o.opts['fname'] is None:
        print "Try %s --help for usage details" % sys.argv[0]
        print "Please enter a file name to log to"
        raise SystemExit, 1
        #sys.exit(1)
    else:
        fname=o.opts['fname']

    print fname

    curses.wrapper(initall)

  
