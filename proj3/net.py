import sys
import socket
from scapy.all import *  # importing without having to specify from the library


def getSysInfo():
    # getting default mac interface hardware address
    host_mac = get_if_hwaddr(conf.iface)
    # host_name = socket.gethostname()
    # host_ip = socket.gethostbyname(host_name)
    host_ip = ARP().psrc
    print(f"Host MAC: {host_mac}")
    print(f"Host IP: {host_ip}")

# if no dest is specified, we assume a broadcast packet


def createEther(dest_addr=None):
    host_ether_header = None  # declaring variable
    if(dest_addr == None):
        host_ether_header = Ether()  # default makes with broadcast address
        host_ether_header.type = 0x806

    else:
        host_ether_header = Ether(dst=dest_addr)
        host_ether_header.type = 0x800
    # ls(host_ether_header) shows all fields within
    if(debug):
        host_ether_header.show()  # alternative way to show header structure

    return host_ether_header

# if none specified, it will be 0.0.0.0)


def createArp(dest_ip_addr=None):
    arp_header = ARP()
    arp_header.pdst = dest_ip_addr
    if(debug):
        arp_header.show()

    return arp_header


def sendRecvARP(arp_packet):
    # send out our assembled arp header
    response, unanswered = srp((arp_packet), timeout=2)
    try:
        # check against our hw address before printing arp
        if(response.res[0][1].sprintf("%Ether.dst%") == get_if_hwaddr(conf.iface)):
            print("Successful ARP Response")
            response.res[0][1].show()
        else:
            print("Did not receive correct ARP response")
    except:
        print("Did not recieve any responses")


if __name__ == "__main__":
    global debug
    debug = 0
    if(len(sys.argv) > 1 and sys.argv[1] == "debug"):
        debug = 1

    getSysInfo()  # this only debugs my sys info rn, but is useful
    eth = createEther()
    arp = createArp("10.0.0.2")

    print("\nAssembled Payload:")
    full_send = eth/arp  # scapy way to assemble the total thing
    full_send.show()

    sendRecvARP(full_send)
