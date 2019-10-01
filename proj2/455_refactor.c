#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <net/if.h>
#include <netinet/ether.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#define BUF_SIZ 65536
#define SEND 0
#define RECV 1

char interfaceName[IFNAMSIZ];
char buf[BUF_SIZ];
unsigned char hw_addr[6];
int debug = 0;
ssize_t numbytes;

int main(int argc, char *argv[]) {
  int mode;
  memset(buf, 0, BUF_SIZ);

  if (argc > 3 && strcmp(argv[3], "debug") == 0) {
    debug = 1;
  }

  int correct = 0;
  if (argc > 1) {
    if (argc >= 3) {
      sscanf(argv[2], "%d.%d.%d.%d", &hw_addr[0], &hw_addr[1], &hw_addr[2],
             &hw_addr[3]);
      correct = 1;
    }

    // IFNAMESIZ defined in net/if.h so ignore lint
    strncpy(interfaceName, argv[1], IFNAMSIZ);
  }

  // console notice for how to run
  if (!correct) {
    fprintf(stderr, "./455_proj2  <InterfaceName>  <DestIP> \n");
    exit(1);
  }

  if (debug) {
    printf("Correct Args\n");
    printf("pre parse argv2: %s\n", argv[2]);
    printf("SENDER IP address: %d:%d:%d:%d\n", hw_addr[0], hw_addr[1],
           hw_addr[2], hw_addr[3]);
  }

  // setting up the receive side of the server

  int sockfd;
  struct ifreq if_idx;
  struct ifreq if_mac;
  int sendLength = 0;
  char sendbuf[BUF_SIZ];
  // ethernet header
  struct ether_header *eh = (struct ether_header *)sendbuf;
  struct sockaddr_ll socket_address;

  // arp header
  struct arp_header *ah;

  // open a socket to listen on
  if ((sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0) {
    perror("socket() failed");
    exit(1);
  }

  // assigning to socket to an interface
  memset(&if_idx, 0, sizeof(struct ifreq));
  strncpy(if_idx.ifr_name, interfaceName, IFNAMSIZ - 1);
  if (ioctl(sockfd, SIOCGIFINDEX, &if_idx) < 0)
    perror("SIOCGIFINDEX");

  // getting mac address of interface
  memset(&if_mac, 0, sizeof(struct ifreq));
  strncpy(if_mac.ifr_name, interfaceName, IFNAMSIZ - 1);

  // modified to print out mac addresses for convienience/debugging
  if (ioctl(sockfd, SIOCGIFHWADDR, &if_mac) == 0) {
    if (debug)
      printf("Getting our mac address: ");
    for (int i = 0; i < 6; ++i) {
      if (debug)
        printf("%02x", (unsigned char)if_mac.ifr_addr.sa_data[i]);
      // setup source mac addy while printing
      eh->ether_shost[i] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[i];
      if (debug) {
        if (i < 5)
          printf(":");
      }
    }
    printf("\n");
  }

  // setup sockaddr_ll
  socket_address.sll_family = PF_PACKET;
  socket_address.sll_protocol = htons(ETH_P_ARP);
  socket_address.sll_ifindex = if_idx.ifr_ifindex;
  socket_address.sll_hatype = ARPHRD_ETHER;
  socket_address.sll_pkttype = 0; // PACKET_OTHERHOST;
  socket_address.sll_halen = 0;
  socket_address.sll_addr[6] = 0x00;
  socket_address.sll_addr[7] = 0x00;
  // not all of this may be completely necessary

  // can throw an interrupt here if wanted

  // wait for incoming packets
  while (1) {
    numbytes = recvfrom(sockfd, buf, BUF_SIZ, 0, NULL, NULL);
    if (numbytes < 0) {
      perror("recvfrom():");
    }

    if (ntohs(eh->h_proto) == ETH_P_ARP) {
			// TODO stopped at ~ 112
    }
  }

  return 0;
}
