from scapy.all import *
import socket
global debug


def sendAck():
    newsock = socket.socket(socket.AF_INET,  # Internet
        socket.SOCK_DGRAM)  # UDP
    newsock.sendto(bytes('end', encoding='UTF-8'), ('172.31.98.12', 5006))
    newsock.close()
    print('sent ack')


if __name__ == "__main__":

    print('Project 4 client')

    UDP_IP = '172.31.98.12'
    UDP_PORT = 5005

    sock = socket.socket(socket.AF_INET,  # Internet
                         socket.SOCK_DGRAM)  # UDP
    sock.bind((UDP_IP, UDP_PORT))
    sock.settimeout(4)
    file = open("output.txt", "a+")
    i = 0
    while True:
        try:
            if i is 10:
                sendAck()
                i = 0
            data, addr = sock.recvfrom(1024)  # buffer size is 1024 bytes
            # print(data.decode('UTF-8'), end='')
            file.write(data.decode('UTF-8'))
            i+=1
        except (socket.timeout):
            print("Socket timeout, closing")
            sendAck()
            break
    

