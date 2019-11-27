# from scapy.all import *
# import socket
# import sys

# global debug

# if __name__ == "__main__":
#     debug = 0
#     file = open('tux.txt','r')
#     lines = file.readlines()
#     if debug:
#         for line in lines:
#             print(line,end="") #establish no newline on end
#     if(len(sys.argv) < 3 and not debug):
#         print("ARGS <ip> <filename>")
    
import socket
from scapy.all import *

def waitForAck():
    print("waiting for ack")
    while(True):
        newsock = socket.socket(socket.AF_INET, # Internet
                    socket.SOCK_DGRAM) # UDP
        newsock.bind(('172.31.98.12', 5006))
        newsock.settimeout(5)
        try: 
            data, addr = newsock.recvfrom(1024) # buffer size is 1024 bytes
            if data.decode('UTF-8') == 'end':
                print(data.decode('UTF-8'))
                print('we acked')
                newsock.close()
                break

        except (socket.timeout):
            print('failed to ack')
            newsock.close()
            exit(1)

if __name__ == "__main__":
    
    if (len(sys.argv)<3):
        exit(1)

    UDP_IP = sys.argv[1]
    filename = sys.argv[2]
    print("Opening file:",filename)
    print("UDP target IP:", UDP_IP)
    # print("UDP target port:", UDP_PORT)
    UDP_PORT = 5005

    file = open(filename,'r')
    lines = file.readlines()
    sock = socket.socket(socket.AF_INET, # Internet
                        socket.SOCK_DGRAM) # UDP

    window = 10 # send only max of 10 packets per window
    sentPackets = 0
    i = 0
    for line in lines:
        if i is 10:
            waitForAck()
            i = 0
        sock.sendto(bytes(line,encoding='UTF-8'), (UDP_IP, UDP_PORT))
        i+=1
    print("completed send")

    waitForAck()
    
            