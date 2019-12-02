import socket
from scapy.all import *

def getIP():
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.connect(("10.0.0.1", 80))
    print(s.getsockname()[0])
    IP = s.getsockname()[0] # get local ip
    s.close()
    return IP

def recvPackets(file,recvIP,port):
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind((recvIP, port))
    i = 0
    try:
        data, addr = sock.recvfrom(1024)
        data = data.decode('UTF-8')
        packetNum = data[0]
        print(f'recieved packet {packet}')
        msg = data[1:]
        file.write(msg)

    except(socket.timeout):
        print('socket timed out, exiting')
        sock.close()
        exit(1)

if __name__ == "__main__":
    recvIP = getIP()
    port = 5005
    output = open('output.txt', 'w+')
    print("started client")
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind((recvIP, port))
    while True:

        data, addr = sock.recvfrom(1024)
        data = data.decode('UTF-8')
        packetNum = data[0]
        msg = data[1:]
        # print(data)
        if data == 'term':
            print('finished')
            sock.close()
            exit(0)
        print(f'recieved packet {packetNum}')
        output.write(msg)
