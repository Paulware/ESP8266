import socket
import msvcrt
import select
 
# windows only
def getKeyboard():
   char = ''
   if msvcrt.kbhit():
      char = msvcrt.getch()
   return char
  
 
clientIp = '192.168.0.255'
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.sendto('findkey', (clientIp, 3333))
print 'Press any keys (1..3) to send commands, q to quit'
while True:
    msg = getKeyboard ()
    if msg != '':
       if msg == 'q':
          break
       else:
          sock.sendto('cmd' + msg, (clientIp,3333))
                     
    data = ''  
    addr = ''   
    i,o,e = select.select ([sock],[],[],0.0001)
    for s in i:
       if s == sock:
          data, addr = sock.recvfrom(1024)      
          if clientIp == '192.168.0.255':
             clientIp = str(addr[0]) # I now know the actual ip address of the device.
             print 'Client discovered at: ' + clientIp

          print "received message:" + data + ' from: ' + str(addr[0])    
          break
sock.close()