import socket
from scapy.all import *


def getIP():
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.connect(("10.0.0.1", 80))
    print(s.getsockname()[0])
    IP = s.getsockname()[0]  # get local ip
    s.close()
    return IP


# doing go back n, with a given buffer window, N start point
def sendPacket(buffer, n, sendIP, port,sock):
    while n < len(buffer):
        data = buffer[n]
        # print(data,end='')
        sock.sendto(bytes(str(n)+data, encoding='UTF-8'), (sendIP, port))
        n += 1

def sendTerm(sendIP,port):
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.sendto(bytes('term', encoding='UTF-8'), (sendIP, port))
    sock.close()

# either we recieve ack, or n position to start at
def recvACK(recvIP, port,sock):
    # print(recvIP)
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
            sendTerm(sendIP,port)
            sock.close()
            exit(1)


def makeWindows(fileLines):
    windows = []
    buffer = []
    maxLines = 10
    for line in lines:
        if len(buffer) == maxLines:
            windows.append(buffer)
            buffer = []

        buffer.append(line)

        if len(buffer) < 10 and line == lines[len(lines)-1]:
            windows.append(buffer)
    return windows


if __name__ == "__main__":
    if (len(sys.argv) < 3):
        exit(1)
    window = 10
    port = 5005
    sendIP = sys.argv[1]
    filename = sys.argv[2]
    recvIP = getIP()  # establish our IP

    file = open(filename, 'r')
    lines = file.readlines()

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind((recvIP, port))
    # sock.settimeout(5)

    buffer = makeWindows(lines)
    termWindow = ['term']
    buffer.append(termWindow)

    sock.settimeout(3)
    for window in buffer:
        n = 0  # start at 0
        while n!= 10:
            sendPacket(window, n, sendIP, port,sock)
            # print('Sent Window')
            # ackNum = 10
            n = recvACK(recvIP, port,sock)
            print(f'Acked {n}')
            time.sleep(.2)
    sendTerm(sendIP,port)
    print('sent term')
