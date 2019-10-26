import socket
from scapy.all import *
import sys

# sudo fuser -k 6653/tcp

# send arp to destination IP, return its HW address
def sendARP(dest_ip):
    eth = Ether(type=0x806, dst="ff:ff:ff:ff:ff:ff")
    arph = ARP(pdst=dest_ip)
    final_packet = eth/arph
    if(debug):
        final_packet.show()
    resp = srp1(final_packet)
    try:
        if(resp.haslayer(ARP)):
            print("received an ARP")
            # print(f"\nHW address for {dest_ip} is {resp[ARP].hwsrc}")
            resp[ARP].show()
            return resp[ARP].hwsrc
    except:
        print("Response timed out")


def sendEthMsg(dest_hw_addr, msg):
    eth = Ether(type=0x800, dst=dest_hw_addr)
    raw_payload = Raw(load=msg)
    eth.add_payload(raw_payload)
    if(debug):
        print("\nsend eth header\n")
        eth.show()
    sendp(eth)


def sendIPMsg(dest_ip, router_ip, msg):

    dest_hw_addr = sendARP(dest_ip)

    eth = Ether(type=0x800, dst=dest_hw_addr)
    iph = IP(dst=dest_ip, ttl=6)
    full_pack = eth/iph

    raw_payload = Raw(load=msg)
    full_pack.add_payload(raw_payload)

    if(debug):
        print("\nsend eth header\n")
        full_pack.show()
    sendp(full_pack)


def sendIPmsg(dest_ip, router_ip, msg):
    '''
    arp our router,
    then send 0x800 msg to router with ip of desired dest
    '''
    # default ether has router as dest, will arp later for verbosity
    eth = Ether(type=0x800)
    iph = IP(dst=dest_ip, ttl=6)
    final_packet = eth/iph
    raw_payload = Raw(load=msg)
    final_packet.add_payload(raw_payload)
    final_packet.show()
    sendp(final_packet)


if __name__ == "__main__":
    global debug
    debug = 1

    # send using IP
    if(sys.argv[1] == "ip"):
        sendIPmsg(sys.argv[2], sys.argv[3], sys.argv[4])
        exit(0)

    # send ethernet message
    if(len(sys.argv) > 2 and sys.argv[1] == "send"):
        dest_hw = sendARP(str(sys.argv[2]))
        sendEthMsg(dest_hw, str(sys.argv[3]))
        exit(0)

    # recv
    if(sys.argv[1] == "recv"):
        our_addr = get_if_hwaddr(conf.iface)
        while(1):

            # use sniff to recieve packets on our default interface
            print("Listening for message")
            msg_pkt = sniff(count=1)

            # ensure the package has our raw payload, and is for us
            if(msg_pkt[0].haslayer(Raw) and msg_pkt[0][Ether].dst == our_addr):
                if(debug):
                    msg_pkt[0].show()
                print("Message Recieved")
                msg_pkt[0][Raw].show()
                print("\n")

    else:
        print("Bad input, exiting")
