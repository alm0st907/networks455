from scapy.all import *
import socket
global debug


def sendAck(buffer):
    newsock = socket.socket(socket.AF_INET,  # Internet
        socket.SOCK_DGRAM)  # UDP
    newsock.sendto(bytes(buffer, encoding='UTF-8'), (UDP_IP, 5007))
    newsock.close()
    print(f'sent ack buf:{buffer}')


if __name__ == "__main__":

    print('Project 4 client')

    global UDP_IP
    UDP_IP = '192.168.0.6'
    UDP_PORT = 5005

    sock = socket.socket(socket.AF_INET,  # Internet
                         socket.SOCK_DGRAM)  # UDP
    sock.bind((UDP_IP, UDP_PORT))
    sock.settimeout(4)
    file = open("output.txt", "a+")
    i = 0
    recvBuffer = ''
    while True:
        try:
            if i is 10:
                sendAck(recvBuffer)
                i = 0
                recvBuffer = ''
            data, addr = sock.recvfrom(1024)  # buffer size is 1024 bytes
            # print(data.decode('UTF-8'), end='')
            if (data.decode('UTF-8') == 'term'):
                print('message received, terminate connection')
                sock.close()
                exit(0)

            file.write(data.decode('UTF-8')[1:])
            recvBuffer += data.decode('UTF-8')[0]
            i+=1
        except (socket.timeout):
            print("Socket timeout, closing")
            sendAck('term')
            break
    

