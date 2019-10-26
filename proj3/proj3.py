import socket
from scapy.all import *
import sys

#send arp to destination IP, return its HW address
def sendARP(dest_ip):
    #grabs our hw addr by default, only need to set arp type
    eth = Ether(type=0x806)
    #set the destination ip, all other fields defaulte
    arph = ARP(pdst=dest_ip)
    #assemble packets toegether
    final_packet = eth/arph

    if(debug):
        final_packet.show()

    #send one packet and wait for answer
    resp = srp1(final_packet)
    try:
        if(resp.haslayer(ARP)):
            print("received an ARP")
            print(f"\nHW address for {dest_ip} is {resp[ARP].hwsrc}")
            return resp[ARP].hwsrc
    except:
        print("Response timed out")

def sendEthMsg(dest_hw_addr,msg):
    eth = Ether(type=0x800, dst = dest_hw_addr)
    raw_payload = Raw(load=msg)
    eth.add_payload(raw_payload)
    if(debug):
        print("\nsend eth header\n")
        eth.show()
    sendp(eth)

if __name__ == "__main__":
    global debug
    debug = 1

    if(len(sys.argv)<2):
        print("Usage:\n\tpython3 proj3 send <ip_addr> <message>")
        print("\tpython3 proj3 recv")
        exit(1)

    #send
    if(len(sys.argv)>2 and sys.argv[1] == "send"):
        dest_hw = sendARP(str(sys.argv[2]))
        sendEthMsg(dest_hw,str(sys.argv[3]))
        exit(0)

    #recv
    if(sys.argv[1] == "recv"):
        our_addr = get_if_hwaddr(conf.iface)
        while(1):

            #use sniff to recieve packets on our default interface
            print("Listening for message")
            msg_pkt = sniff(count=1)

            #ensure the package has our raw payload, and is for us
            if(msg_pkt[0].haslayer(Raw) and msg_pkt[0][Ether].dst == our_addr):
                if(debug):
                    msg_pkt[0].show()
                print("Message Recieved")
                msg_pkt[0][Raw].show()
                print("\n")    

    else:
        print("Bad input, exiting")

    