import socket
from scapy.all import *
def sendSnippet():
    UDP_IP = "192.168.1.86"
    UDP_PORT = 5005
    MESSAGE = "fuck you hahn and fuck you jeff"

    print("UDP target IP:", UDP_IP)
    print("UDP target port:", UDP_PORT)
    print("message:", MESSAGE)

    sock = socket.socket(socket.AF_INET, # Internet
                        socket.SOCK_DGRAM) # UDP
    sock.sendto(MESSAGE, (UDP_IP, UDP_PORT))

def recvSnippet():
    UDP_IP = "192.168.1.216"
    UDP_PORT = 5005

    sock = socket.socket(socket.AF_INET, # Internet
                        socket.SOCK_DGRAM) # UDP
    sock.bind((UDP_IP, UDP_PORT))

    while True:
        data, addr = sock.recvfrom(1024) # buffer size is 1024 bytes
        print ("received message:", data)
        # Packet.__class__(bytes(data))
        # data.Show()
        pckt=Raw(data)
        pckt.show()