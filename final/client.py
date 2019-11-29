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
        newsock.bind((UDP_IP, 5007))
        newsock.settimeout(5)
        try: 
            data, addr = newsock.recvfrom(1024) # buffer size is 1024 bytes
            if data.decode('UTF-8') == '0123456789' :
                print('we acked')
                newsock.close()
                break
            else:
                print(f'expected: {buffer}\nrecieved: {data.decode("UTF-8")}')

        except (socket.timeout):
            print('failed to ack')
            newsock.close()
            exit(1)

if __name__ == "__main__":
    
    if (len(sys.argv)<3):
        exit(1)
    global UDP_IP
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
    packetBuffer = dict()

    for line in lines:
        if i is 10:
            # print(packetBuffer) # so we have a dict of packets to determine which ones
            waitForAck()
            i = 0
            packetBuffer = dict()
        sock.sendto(bytes(str(i)+line,encoding='UTF-8'), (UDP_IP, UDP_PORT))
        packetBuffer[i]=line
        i+=1

    sock.sendto(bytes('term',encoding='UTF-8'), (UDP_IP, UDP_PORT))
    print("completed send")

    # waitForAck()
    
            