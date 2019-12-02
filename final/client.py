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

def recvACK(sock):
    # print(recvIP)
    # sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    # sock.bind((recvIP, port))
    sock.settimeout(5)
    try:
        data, addr = sock.recvfrom(1024)
        data = data.decode('UTF-8')
        # sock.close()
        if data == 'ACK':
            return 10  # acked our full window
        else:
            return int(data)
    except(socket.timeout):
        print('socket timed out, exiting')
        sock.close()
        exit(1)

def sendACK(sendIP,port,ack):
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    # ack = f'{ack}'.encode("UTF-8")
    sock.sendto(bytes(str(ack),encoding='UTF-8'), (sendIP, port))
    sock.close()


if __name__ == "__main__":
    recvIP = getIP()
    port = 5005
    output = open('output.txt', 'w+')
    print("started client")
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind((recvIP, port))
    socket.timeout(5)
    nextPacket = 0
    while True:

        data, addr = sock.recvfrom(1024)
        data = data.decode('UTF-8')
        msg = data[1:]
        # print(data)
        if data == 'term':
            print('finished')
            sock.close()
            exit(0)
        packetNum = int(data[0])
        print(f'recieved packet {packetNum}')
        # output.write(msg)
        if nextPacket == packetNum:
            output.write(msg)

            #set number and be ready to loopback
            if packetNum == 9:
                nextPacket = 0
                sendACK(addr[0],port,'ACK')
            else:
                nextPacket = packetNum + 1
        else:
            print('Dropped packet')
            print(nextPacket)
            sendACK(addr[0],port,nextPacket)
            