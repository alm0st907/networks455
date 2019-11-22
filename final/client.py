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

if __name__ == "__main__":
    
    UDP_IP = "192.168.1.86"
    UDP_PORT = 5005
    MESSAGE = "fuck you hahn and fuck you jeff"

    print("UDP target IP:", UDP_IP)
    print("UDP target port:", UDP_PORT)
    print("message:", MESSAGE)

    sock = socket.socket(socket.AF_INET, # Internet
                        socket.SOCK_DGRAM) # UDP
    sock.sendto(MESSAGE, (UDP_IP, UDP_PORT))