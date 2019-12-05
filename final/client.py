'''
garrett rudisill
sending side of the project
'''
import socket
import sys

# python way to cross platform get IP
def getIP():
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.connect(("10.0.0.1", 80))
    print(s.getsockname()[0])
    IP = s.getsockname()[0]  # get local ip
    s.close()
    return IP


# doing go back n, with a given buffer window, N start point
def sendPacket(buffer, n, sendIP, port,sock):
    windowSize = len(buffer)
    while n < windowSize:
        data = buffer[n]
        # print(data,end='')
        sock.sendto(bytes(str(n)+str(windowSize-1)+data, encoding='UTF-8'), (sendIP, port))
        n += 1

#shortcut to just send a termination signal
def sendTerm(sendIP,port):
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.sendto(bytes('term', encoding='UTF-8'), (sendIP, port))
    sock.close()


# either we recieve ack, or n position to start at
def recvACK(recvIP, port,sock):
    # print(recvIP)
    data = None
    sock.settimeout(6) #no response for 3 seconds, connection is borked and we timeout
    while (data is None) or (data is '') or (data is '\x00'):
        try:
            data, addr = sock.recvfrom(1024)
            data = data.decode('UTF-8')
            # sock.close()
            if data == 'ACK':
                return 10  # acked our full window

            if data == '' or data == '\x00': pass
            else:
                # sock.settimeout(0) #remove the timeout 
                return int(data) #returning where we need to restart at
        except(socket.timeout):
            print('socket timed out, exiting')
            sendTerm(sendIP,port)
            sock.close()
            exit(1)


# take our file and split into 10 packet windows
# returns a list of windows with up to 10 packets

def makeWindows(fileLines):
    windows = []
    buffer = []
    maxLines = 10
    for line in lines:
        if len(buffer) == maxLines:
            windows.append(buffer)
            buffer = []

        buffer.append(line)
        #if our window has less than 10 packets, making sure it gets added into the buffer of windows
        if len(buffer) < 10 and line == lines[len(lines)-1]:
            windows.append(buffer)
    return windows


if __name__ == "__main__":
    if (len(sys.argv) < 3): #if we dont have ip/file args we exit
        print("run as:\npython3 client.py <ip> <filename>")
        exit(1)
    packetWindow = 10
    port = 5005
    sendIP = sys.argv[1]
    filename = sys.argv[2]
    recvIP = getIP()  # establish our IP

    #parse file into list of lines
    file = open(filename, 'r')
    lines = file.readlines()

    #create and bind our UDP socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind((recvIP, port))
    # sock.settimeout(5)

    #make our windows from the lines
    buffer = makeWindows(lines)
    # termWindow = ['term'] #redundant window with term to assist if term message gets dropped
    # buffer.append(termWindow)
    lastAck = 0

    sock.settimeout(6) # timeout of 3 seconds so we eventually term the client
    #iterate through windows
    for window in buffer:
        n = 0  # start at 0
        # we only advance to next window when current window has been fully acked
        windowSize = len(window)
        while n < windowSize:
            #send our window, initially starting at n position of 0 (initial send)
            sendPacket(window, n, sendIP, port,sock)
            # print('Sent Window')
            
            # n is either the ACK of 10 (we got the full window)
            # otherwise n is what the expected packet was and not received
            # this will loop to resend window from that index on (packet SN's and buffer are both 0 indexed)
            n = recvACK(recvIP, port,sock)
            if n < lastAck :
                print('implicit ACKED, advance window')
                lastAck = 0
                break #we dropped the ack but we can advance
                

            if n < 10:
                print(f'{n}NACK') #debugging what the ack was
                lastAck = n

            else:
                lastAck = 0
                print('ACKED, advance window')


    #send a termination since we are done sending windows
    terminationFloodCount = 0
    while(terminationFloodCount<10):
        sendTerm(sendIP,port)
        terminationFloodCount+=1
    print('sent term')
