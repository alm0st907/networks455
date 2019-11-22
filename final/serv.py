from scapy.all import *
import socket
global debug
if __name__ == "__main__":

    print('Project 4 client')

    UDP_IP = "192.168.1.216"
    UDP_PORT = 5005

    sock = socket.socket(socket.AF_INET, # Internet
                        socket.SOCK_DGRAM) # UDP
    sock.bind((UDP_IP, UDP_PORT))

    while True:
        data, addr = sock.recvfrom(1024) # buffer size is 1024 bytes
        print ("received message:", data)
        pckt=Raw(data)
        pckt.show()

