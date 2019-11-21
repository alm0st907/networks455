from scapy.all import *
import socket
global debug
if __name__ == "__main__":
    print('Project 4 client')

    ipAddress = "10.0.0.1"
    udpPort = 6969

    sock = socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
    sock.bind((ipAddress,udpPort))
    while True:
        data, addr = sock.recvfrom()