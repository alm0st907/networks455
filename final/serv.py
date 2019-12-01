from scapy.all import *
import socket
global debug

#sudo mn --mac --switch ovsk --controller=remote,ip=0.0.0.0,port=6633

def sendAck(buffer):
    newsock = socket.socket(socket.AF_INET,  # Internet
        socket.SOCK_DGRAM)  # UDP
    buffer = ''.join(sorted(set(buffer)))
    newsock.sendto(bytes(buffer, encoding='UTF-8'), ("10.0.0.1", 5007))
    newsock.close()
    missing_packets = ''
    print(f'sent ack buf:{buffer}')
    for c in '0123456789':
        if c not in buffer:
            missing_packets += c
    print(f'missing packets {missing_packets}')

#function to encapsulate recieving process
def recvPackets():
    pass

#need to know how many packets are expected
def getMissingPackets(missingPackets, packetBuffer):
    pass


if __name__ == "__main__":

    print('Project 4 client')

    # establish local IP
    global UDP_IP
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.connect(("10.0.0.1", 80))
    # print(s.getsockname()[0])
    UDP_IP = s.getsockname()[0] # get local ip
    s.close()

    UDP_PORT = 5005

    #setup for sending
    sock = socket.socket(socket.AF_INET,  # Internet
                         socket.SOCK_DGRAM)  # UDP
    sock.bind((UDP_IP, UDP_PORT))
    sock.settimeout(2)
    file = open("output.txt", "a+")
    i = 0
    recvBuffer = '' #the ack numbers from packets into a string
    recvPacketsBuffer = {} #packets recieved in a dictionary, key is the ack #
    while True:
        try:
            if i is 10:
                sendAck(recvBuffer)
                i = 0
                recvBuffer = ''
                for x in sorted(recvPacketsBuffer.keys()):
                    file.write(recvPacketsBuffer[x])
                recvPacketsBuffer = {}

            data, addr = sock.recvfrom(1024)  # buffer size is 1024 bytes
            # print(data.decode('UTF-8'), end='')
            if (data.decode('UTF-8') == 'term'):
                print('message received, terminate connection')
                sock.close()
                exit(0)
            recvPacketsBuffer[data.decode('UTF-8')[0]] = data.decode('UTF-8')[1:]
            recvBuffer += data.decode('UTF-8')[0]

            i+=1
        except (socket.timeout):
            print("Socket timeout, closing")
            print("flushing buffer")

            #flushing the file in order of keys
            for x in sorted(recvPacketsBuffer.keys()):
                file.write(recvPacketsBuffer[x])

            sendAck(recvBuffer)
            break
    

