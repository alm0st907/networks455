'''
garrett rudisill
receive side of project
'''
import socket
from scapy.all import *

def getIP():
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.connect(("10.0.0.1", 80))
    print(s.getsockname()[0])
    IP = s.getsockname()[0] # get local ip
    s.close()
    return IP

#may need to pass in premade socket for this to work right rather than creating a socket, sending the ack then closing
def sendACK(sendIP,port,ack,sock):
    # sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.sendto(bytes(str(ack),encoding='UTF-8'), (sendIP, port))


if __name__ == "__main__":
    if (len(sys.argv) < 2): #if we dont have file args we exit
        print("run as:\npython3 server.py <filename>")
        exit(1)
    
    filename = sys.argv[1]
    recvIP = getIP()
    port = 5005

    #open our output file to write, will create new file if already exists
    output = open(filename, 'w+')
    print("started client")

    #create and bind the socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind((recvIP, port))
    sock.settimeout(3)
    #expected SN of packet to start
    nextPacket = 0
    while True:
        try:
            data, addr = sock.recvfrom(1024)
            data = data.decode('UTF-8') #decode bytes to regular string
            msg = data[1:] #strip off SN to have the line to write
            # print(data)
            
            # server termination clause, whether from the explict term or the window term
            if data =='term' or data[1:] == 'term':
                print('finished')
                sock.close()
                exit(0)
            
            #if its not term, we strip the SN from packet to check if its the expected packet
            packetNum = int(data[0])
            print(f'expected {nextPacket} recieved packet {packetNum}')
            # output.write(msg)

            #if the packet is our expected packet, write msg to file, advance the next packet expected
            if nextPacket == packetNum:
                output.write(msg)
                nextPacket+=1     
            #if the packet is not expeceted, we nack the packet we were expecting so the client can resend from that point on
)

        #catch timeout exception
        except(socket.timeout):

            # if our next packet hits 10, we have recieved all packets (sn is 0-9 for 10 packets total)
            # reset to next packet of 0, send ACK to client to advance the window
            if nextPacket == 10:
                print('ACK')
                nextPacket = 0
                sendACK(addr[0],port,'ACK',sock
            else:
                print(f'{nextPacket}NACK')
                print(nextPacket)
                sendACK(addr[0],port,nextPacket,sock)
                # time.sleep(1)
   