import socket
from scapy.all import *

#sudo mn --mac --switch ovsk --controller=remote,ip=0.0.0.0,port=6633

def waitForAck(ack_sequence):
    print("waiting for ack")
    while(True):
        newsock = socket.socket(socket.AF_INET, # Internet
                    socket.SOCK_DGRAM) # UDP
        newsock.bind((RECV_UDP_IP, 5007))
        newsock.settimeout(5)
        try: 
            data, addr = newsock.recvfrom(1024) # buffer size is 1024 bytes
            print(addr)
            data = data.decode('UTF-8')
            if data == ack_sequence :
                print('we acked')
                newsock.close()
                break
            else:
                print(f'expected: {ack_sequence}\nrecieved: {data}')
                missing_packets = ''
                for c in ack_sequence:
                    if c not in data:
                        missing_packets += c
                
                print(f'Need to resend packets {missing_packets}')

        except (socket.timeout):
            print('failed to ack')
            newsock.close()
            exit(1)

def sendMissingPackets():
    pass

if __name__ == "__main__":
    
    if (len(sys.argv)<3):
        exit(1)
    global DEST_UDP_IP
    global RECV_UDP_IP
    DEST_UDP_IP = sys.argv[1]
    filename = sys.argv[2]

    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.connect(("10.0.0.1", 80))
    print(s.getsockname()[0])
    RECV_UDP_IP = s.getsockname()[0] # get local ip
    s.close()

    print("Opening file:",filename)
    print("UDP target IP:", DEST_UDP_IP)
    # print("UDP target port:", UDP_PORT)
    UDP_PORT = 5005

    file = open(filename,'r')
    lines = file.readlines()
    sock = socket.socket(socket.AF_INET, # Internet
                        socket.SOCK_DGRAM) # UDP

    window = 10 # send only max of 10 packets per window
    sentPackets = 0
    i = 0
    packetBuffer = dict()
    windowAcqSequence = '' #packets ack sequence we are sending, eg 0-(number of packets -1) with a max of 10

    #need to send how many packets expected in a given window if less than 10
    for line in lines:
        if i is 10:
            # print(packetBuffer) # so we have a dict of packets to determine which ones
            waitForAck(windowAcqSequence)
            i = 0
            windowAcqSequence
            packetBuffer = dict()
        sock.sendto(bytes(str(i)+line,encoding='UTF-8'), (DEST_UDP_IP, UDP_PORT))
        packetBuffer[i]=line
        windowAcqSequence += str(i)
        i+=1

    # sock.sendto(bytes('term',encoding='UTF-8'), (DEST_UDP_IP, UDP_PORT))
    print("completed send")

    # waitForAck()
    
            