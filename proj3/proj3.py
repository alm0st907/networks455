# Garrett Rudisill
# WSU ID 11461816
import socket
from scapy.all import *
# removes Ether,IP,ARP lint errors, but is already included through scapy.all
from scapy.layers.l2 import Ether, ARP
from scapy.layers.inet import IP
import sys
import netifaces
from netaddr import *

# sudo fuser -k 6653/tcp

# displays system info
def displayInfo():
    interface = conf.iface  # default intesrfaces
    print(f"Using default interface pulled: {conf.iface}")
    info = netifaces.ifaddresses(interface)
    print(f"Interface Info: {info[netifaces.AF_INET][0]}")
    return


def determineNetmask(dest_ip):

    interface = conf.iface  # default intesrfaces
    info = netifaces.ifaddresses(interface)
    if(debug):
        print(f"Interface Info: {info[netifaces.AF_INET][0]}")
        print(f"Our Netmask: {info[netifaces.AF_INET][0]['netmask']}")

    my_ip = info[netifaces.AF_INET][0]['addr']
    my_netmask = info[netifaces.AF_INET][0]['netmask']

    if (IPNetwork(my_ip+'/'+my_netmask) == IPNetwork(dest_ip+'/'+my_netmask)):
        print("IP within our subnet, ARP IP")
        return True
    else:
        print("IP outside subnet, ARP Router")
        return False

# send arp to destination IP, return its HW address
def sendARP(dest_ip):

    eth = Ether(type=0x806, dst="ff:ff:ff:ff:ff:ff",
                src=get_if_hwaddr(conf.iface))
    arph = ARP(pdst=dest_ip)
    final_packet = eth/arph
    if(debug):
        final_packet.show()
    resp = srp1(final_packet, iface=conf.iface)
    try:
        if(resp.haslayer(ARP)):
            print("received an ARP")
            # print(f"\nHW address for {dest_ip} is {resp[ARP].hwsrc}")
            if(debug):
                resp[ARP].show()
            return resp[ARP].hwsrc
    except:
        print("Response timed out")


def sendEthMsg(dest_hw_addr, msg):
    eth = Ether(type=0x800, dst=dest_hw_addr, src=get_if_hwaddr(conf.iface))
    raw_payload = Raw(load=str(msg))
    eth.add_payload(raw_payload)
    if(debug):
        print("\nsend eth header\n")
        eth.show()
    # send packet using socked on conf.iface, which is default interface (h1x1-eth0, etc)
    sendp(eth, iface=conf.iface)


def sendIPmsg(dest_ip, router_ip, msg):
    '''
    arp our router,
    then send 0x800 msg to router with ip of desired dest
    '''

    # compare our IP's for if we need to arp router, or IP
    dst_hw_addr = None
    if(determineNetmask(dest_ip)):
        dst_hw_addr = sendARP(dest_ip)
    # arp  our router for the HW addr
    else:
        dst_hw_addr = sendARP(router_ip)
    # send
    eth = Ether(type=0x800, dst=dst_hw_addr, src=get_if_hwaddr(conf.iface))
    # pulled protocol numbers from https://www.iana.org/assignments/protocol-numbers/protocol-numbers.xhtml
    # 253 is for testing and experimentation, which silenced some wireshark oddities
    iph = IP(dst=dest_ip, ttl=6, proto=253)

    final_packet = eth/iph
    raw_payload = Raw(load=str(msg))
    final_packet.add_payload(raw_payload)
    if(debug):
        final_packet.show()

    # clear checksum and let scapy recalc
    del final_packet[IP].chksum
    final_packet = final_packet.__class__(bytes(final_packet))
    print(f"Checksum calculated by Scapy: {hex(final_packet[IP].chksum)}")

    # send packet using socked on conf.iface, which is default interface (h1x1-eth0, etc)
    sendp(final_packet, iface=conf.iface)



if __name__ == "__main__":
    global debug
    debug = 0
    # send using IP
    if(len(sys.argv) > 4 and sys.argv[1] == "send"):
        displayInfo()
        sendIPmsg(sys.argv[2], sys.argv[3], sys.argv[4])
        exit(0)

    # # send ethernet message
    # if(len(sys.argv) > 2 and sys.argv[1] == "send"):
    #     displayInfo()
    #     dest_hw = sendARP(str(sys.argv[2]))
    #     sendEthMsg(dest_hw, str(sys.argv[3]))
    #     exit(0)

    # recv
    if(len(sys.argv) > 1 and sys.argv[1] == "recv"):
        our_addr = get_if_hwaddr(conf.iface)
        displayInfo()
        while(1):

            # use sniff to recieve packets on our default interface
            print("Listening for message")
            msg_pkt = sniff(count=1, filter="ip", iface=conf.iface)
            # show because sometimes the packet is getting clobbered
            # msg_pkt[0].show()

            # ensure the package has our raw payload, and is for us
            if(msg_pkt[0].haslayer(Raw) and msg_pkt[0][Ether].dst == our_addr):
                if(debug):
                    msg_pkt[0].show()
                print("Message Recieved")
                print(f"Checksum {hex(msg_pkt[0][IP].chksum)}")
                msg_pkt[0][Raw].show()
                print("\n")

    else:
        print("\nUsage:")
        print("python3 proj3 recv")
        print("python3 proj3 send <dest_ip> <router_ip> <message>")
        print("Interfaces are automatically pulled from system programmatically\nProgram depends on Scapy,netifaces,netaddr")
