#include <arpa/inet.h>
#include <linux/if_ether.h>  // ETH_P_ARP = 0x0806
#include <linux/if_packet.h> // struct sockaddr_ll
#include <net/if.h>          // struct ifreq
#include <netdb.h>           // struct addrinfo
#include <netinet/ether.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>  // macro ioctl is defined
#include <sys/socket.h> // needed for socket()
#define PROTO_ARP 0x0806
#define ETH2_HEADER_LEN 14
#define HW_TYPE 1
#define PROTOCOL_TYPE 0x800
#define MAC_LENGTH 6
#define IPV4_LENGTH 4
#define ARP_REQUEST 0x01
#define ARP_REPLY 0x02
#define BUF_SIZ 65536
#define SEND 0
#define RECV 1

int debug = 0;

struct arp_header {
  uint16_t ar_hrd;
  uint16_t ar_pro;
  unsigned char ar_hln;
  unsigned char ar_pln;
  uint16_t ar_op;
  unsigned char ar_sha[MAC_LENGTH];
  unsigned char ar_sip[IPV4_LENGTH];
  unsigned char ar_tha[MAC_LENGTH];
  unsigned char trg_ip[IPV4_LENGTH];
};

int main(int argc, char *argv[]) {
  int mode;
  if (argc > 3 && strcmp(argv[3], "debug") == 0) {
    debug = 1;
  }

  int correct = 0;
  if (argc > 1) {
    if (argc >= 3) {
      correct = 1;
    }
  }

  // console notice for how to run
  if (!correct) {
    fprintf(stderr, "./455_proj2  <InterfaceName>  <DestIP> \n");
    exit(1);
  }

  int sockfd;
  unsigned char buffer[BUF_SIZ];
  unsigned char trg_ip[IPV4_LENGTH]; //arp target ip
  unsigned char src_ip[IPV4_LENGTH]; //source ip
  struct ifreq ifr_ip, if_idx;
  struct ethhdr *send_req = (struct ethhdr *)buffer; //set our buffers to be ethhdr's 
  struct ethhdr *rcv_resp = (struct ethhdr *)buffer;
  struct arp_header *arp_req = (struct arp_header *)(buffer + ETH2_HEADER_LEN); //double set so its easy to distinguish between sent and response
  struct arp_header *arp_resp = (struct arp_header *)(buffer + ETH2_HEADER_LEN);
  struct sockaddr_ll socket_address;
  int index, ret, length = 0, ifindex; //basic fields for sockets/etc
  struct addrinfo *res; //for setting ip
  memset(buffer, 0, BUF_SIZ);
  /*open socket*/
  sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
  if (sockfd == -1) {
    perror("socket():");
    exit(1);
  }
  strcpy(ifr_ip.ifr_name, argv[1]);

  //get index
  if (ioctl(sockfd, SIOCGIFINDEX, &ifr_ip) == -1) {
    perror("SIOCGIFINDEX");
    exit(1);
  }
  ifindex = ifr_ip.ifr_ifindex;
  printf("interface index is %d\n", ifindex);

 //get mac address
  if (ioctl(sockfd, SIOCGIFHWADDR, &ifr_ip) == -1) {
    perror("SIOCGIFINDEX");
    exit(1);
  }
  close(sockfd);

  int status;
  //getting info for source ip
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  memset(&if_idx, 0, sizeof(struct ifreq));
  strncpy(if_idx.ifr_name, argv[1], IF_NAMESIZE - 1);
  if (ioctl(sockfd, SIOCGIFADDR, &if_idx) < 0)
    perror("SIOCGIFADDR");
  close(sockfd);

  //convert and copy
  strcpy(src_ip, inet_ntoa(((struct sockaddr_in *)&if_idx.ifr_addr)->sin_addr));

  // Source IP address
  if ((status = inet_pton(AF_INET, src_ip, &arp_req->ar_sip)) != 1) {
    fprintf(stderr,
            "inet_pton() failed for source IP address.\nError message: %s",
            strerror(status));
    exit(EXIT_FAILURE);
  }
  // alternative way to get the ip address back
  // Resolve target using getaddrinfo().
  strcpy(trg_ip, argv[2]);
  if ((status = getaddrinfo(trg_ip, NULL, NULL, &res)) != 0) {
    fprintf(stderr, "getaddrinfo() failed\n");
    exit(EXIT_FAILURE);
  }
  struct sockaddr_in *ipv4 = (struct sockaddr_in *)res->ai_addr;
  memcpy(&arp_req->trg_ip, &ipv4->sin_addr, 4 * sizeof(uint8_t));

  //setting fields of of requests
  for (index = 0; index < 6; index++) {

    send_req->h_dest[index] = (unsigned char)0xff;
    arp_req->ar_tha[index] = (unsigned char)0x00;
    /* Filling the source  mac address in the header*/
    send_req->h_source[index] = (unsigned char)ifr_ip.ifr_hwaddr.sa_data[index];
    arp_req->ar_sha[index] = (unsigned char)ifr_ip.ifr_hwaddr.sa_data[index];
    socket_address.sll_addr[index] =
        (unsigned char)ifr_ip.ifr_hwaddr.sa_data[index];
  }
  printf(
      "Successfully got interface MAC address: %02X:%02X:%02X:%02X:%02X:%02X\n",
      send_req->h_source[0], send_req->h_source[1], send_req->h_source[2],
      send_req->h_source[3], send_req->h_source[4], send_req->h_source[5]);
  printf(" arp_req MAC address: %02X:%02X:%02X:%02X:%02X:%02X\n",
         arp_req->ar_sha[0], arp_req->ar_sha[1], arp_req->ar_sha[2],
         arp_req->ar_sha[3], arp_req->ar_sha[4], arp_req->ar_sha[5]);
  printf("socket_address MAC address: %02X:%02X:%02X:%02X:%02X:%02X\n",
         socket_address.sll_addr[0], socket_address.sll_addr[1],
         socket_address.sll_addr[2], socket_address.sll_addr[3],
         socket_address.sll_addr[4], socket_address.sll_addr[5]);

  //prep socket address
  socket_address.sll_family = AF_PACKET;
  socket_address.sll_protocol = htons(ETH_P_ARP);
  socket_address.sll_ifindex = ifindex;
  socket_address.sll_hatype = htons(ARPHRD_ETHER);
  socket_address.sll_pkttype = (PACKET_BROADCAST);
  socket_address.sll_halen = MAC_LENGTH;
  socket_address.sll_addr[6] = 0x00;
  socket_address.sll_addr[7] = 0x00;

  //send request protocol
  send_req->h_proto = htons(ETH_P_ARP);

  //set the arp header
  arp_req->ar_hrd = htons(HW_TYPE);
  arp_req->ar_pro = htons(ETH_P_IP);
  arp_req->ar_hln = MAC_LENGTH;
  arp_req->ar_pln = IPV4_LENGTH;
  arp_req->ar_op = htons(ARP_REQUEST);

  // Submit request for a raw socket descriptor.
  if ((sockfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0) {
    perror("socket() failed ");
    exit(EXIT_FAILURE);
  }

  ret = sendto(sockfd, buffer, 42, 0, (struct sockaddr *)&socket_address,
               sizeof(socket_address));
  if (ret == -1) {
    perror("sendto():");
    exit(1);
  }

  printf("\n");
  memset(buffer, 0x00, BUF_SIZ);
  while (1) {
    length = recvfrom(sockfd, buffer, BUF_SIZ, 0, NULL, NULL);
    if (length == -1) {
      perror("recvfrom():");
      exit(1);
    }

    //validating the arp reply
    if (htons(rcv_resp->h_proto) == PROTO_ARP) {
      if( arp_resp->ar_op == ARP_REPLY )
      printf("RECEIVED ARP RESP len=%d \n", length);

      // print this one
      printf("MAC learned from ARP :");
      for (index = 0; index < 6; index++) {
        if (index < 5) {

          printf(" %02X:", arp_resp->ar_sha[index]);
        } else {
          printf(" %02X", arp_resp->ar_sha[index]);
        }
      }

      // check against this one
      printf("\nOur MAC (intended reciever for ARP) :");
      for (index = 0; index < 6; index++) {
        if ((unsigned char)ifr_ip.ifr_hwaddr.sa_data[index] ==
                arp_resp->ar_tha[index] &&
            index < 5)
          printf(" %02X :", arp_resp->ar_tha[index]);
        else {

          printf(" %02X", arp_resp->ar_tha[index]);
        }
      }

      printf("\n");
      break;
    }
  }
  return 0;
}
