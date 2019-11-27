from scapy.all import *
import socket
global debug
if __name__ == "__main__":

    print('Project 4 client')

    UDP_IP = '172.31.98.12'
    UDP_PORT = 5005

    sock = socket.socket(socket.AF_INET,  # Internet
                         socket.SOCK_DGRAM)  # UDP
    sock.bind((UDP_IP, UDP_PORT))
    sock.settimeout(3)
    file = open("output.txt", "a+")
    while True:
        try:
            data, addr = sock.recvfrom(1024)  # buffer size is 1024 bytes
            print(data.decode('UTF-8'), end='')
            file.write(data.decode('UTF-8'))
        except (socket.timeout):
            print("Socket timeout, closing")
            sock = socket.socket(socket.AF_INET,  # Internet
                                 socket.SOCK_DGRAM)  # UDP
            sock.sendto(bytes('end', encoding='UTF-8'), ('172.31.98.12', 5006))
            print('sent ack')
            exit(0)
        # finally:
        #     print('finally')
        #     sock = socket.socket(socket.AF_INET,  # Internet
        #                          socket.SOCK_DGRAM)  # UDP
        #     sock.sendto(bytes('end', encoding='UTF-8'), ('172.31.98.12', 5006))
