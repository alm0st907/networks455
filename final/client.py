import socket
from scapy.all import *

def getIP():
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.connect(("10.0.0.1", 80))
    print(s.getsockname()[0])
    IP = s.getsockname()[0] # get local ip
    s.close()
    return IP

def recvACK(sock):
    # print(recvIP)
    # sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    # sock.bind((recvIP, port))
    # sock.settimeout(5)
    data = None
    while (data is None) or (data is '') or (data is '\x00'):
        try:
            data, addr = sock.recvfrom(1024)
            data = data.decode('UTF-8')
            # sock.close()
            if data == 'ACK':
                return 10  # acked our full window

            if data == '' or data == '\x00': pass
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
    nextPacket = 0
    while True:

        data, addr = sock.recvfrom(1024)
        data = data.decode('UTF-8')
        msg = data[1:]
        # print(data)
        if data =='term' or data[1:] == 'term':
            print('finished')
            sock.close()
            exit(0)
        packetNum = int(data[0])
        print(f'expected {nextPacket} recieved packet {packetNum}')
        # output.write(msg)
        if nextPacket == packetNum:
            output.write(msg)
            nextPacket+=1     
        else:
            print('Dropped packet')
            print(nextPacket)
            sendACK(addr[0],port,nextPacket)
            # time.sleep(1)

            #set number and be ready to loopback
        if nextPacket == 10:
            nextPacket = 0
            sendACK(addr[0],port,'ACK')