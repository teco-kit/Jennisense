import serial

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
  

def main():
#  ser = serial.Serial('COM75', 1000000)
  ser = serial.Serial('COM48', 1000000, parity=serial.PARITY_NONE, stopbits=1, bytesize=8, rtscts=0, dsrdtr=0)
  while 1:
    a = "("
    for i in range(0, 16):
      a += str(ord(ser.read()))
      if i != 15:
        a += ":"
    a += ")"

    for n in range(0, 11):
      node = ser.read(2)
      a += "(" + con_com(ord(node[0]), ord(node[1]))

      data_len = ord(ser.read())
      if data_len == 1:
        a += "," + str(ord(ser.read())) + ")"
      elif data_len == 2:
        data = ser.read(2)
        num = (ord(data[0]) << 8) | ord(data[1])
        if num > 0x7fff:
          num -= 0x10000
        a += "," + str(num) + ")"
      else:
        data = ser.read(6)
        num = (ord(data[0]) << 40) | (ord(data[1]) << 32) | (ord(data[2]) << 24) | (ord(data[3]) << 16) | (ord(data[4]) << 8) | (ord(data[5]))
        a += "," + str(num)
        data = ser.read(2)
        num = (ord(data[0]) << 8) | ord(data[1])
        a += "," + str(num) + ")"

    a += "\n"
        
    print a
  return 
 
 
if __name__ == '__main__':
  main()
