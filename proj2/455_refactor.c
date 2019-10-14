//Garrett Rudisill
#include <arpa/inet.h>
#include <linux/if_ether.h>  // ETH_P_ARP = 0x0806
#include <linux/if_packet.h> // struct sockaddr_ll
// #include <linux/if_arp.h> //contains ARPHDR_ETHER but creates a double define
#include <net/if.h>          // struct ifreq
#include <netdb.h>           // struct addrinfo
#include <netinet/ether.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>  // macro ioctl is defined
#include <sys/socket.h> // needed for socket()
#define HW_TYPE 1
#define MAC_LENGTH 6     //mac address length
#define IPV4_LENGTH 4    //ipv4 have 4 len addresses
#define ARP_REQUEST 0x01 // macros for request/reply
#define ARP_REPLY 0x02
#define BUF_SIZ 65536 //buf from previous assignment "max packet"
#define SEND 0
#define RECV 1

int debug = 0;

//arp header given from assignment
struct arp_header
{
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

int main(int argc, char *argv[])
{
  int mode;
  if (argc > 3 && strcmp(argv[3], "debug") == 0)
  {
    debug = 1;
  }

  int correct = 0;
  if (argc > 1)
  {
    if (argc >= 3)
    {
      correct = 1;
    }
  }

  // console notice for how to run
  if (!correct)
  {
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
  struct arp_header *arp_req = (struct arp_header *)(buffer + ETH_HLEN); //double set so its easy to distinguish between sent and response
  struct arp_header *arp_resp = (struct arp_header *)(buffer + ETH_HLEN);
  struct sockaddr_ll socket_address;
  int ret, length = 0, ifindex; //basic fields for sockets/etc
  struct addrinfo *res;                //for setting ip
  memset(buffer, 0, BUF_SIZ);
  /*open socket*/
  sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
  if (sockfd == -1)
  {
    perror("socket():");
    exit(1);
  }

  //interface name to ifr
  strcpy(ifr_ip.ifr_name, argv[1]);

  //get index
  if (ioctl(sockfd, SIOCGIFINDEX, &ifr_ip) == -1)
  {
    perror("SIOCGIFINDEX");
    exit(1);
  }
  //store our index for later since weird memory issues and we need this for the socket address
  ifindex = ifr_ip.ifr_ifindex;
  printf("interface index is %d\n", ifindex);

  //get mac address
  if (ioctl(sockfd, SIOCGIFHWADDR, &ifr_ip) == -1)
  {
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
  if ((status = inet_pton(AF_INET, src_ip, &arp_req->ar_sip)) != 1)
  {
    perror("inet_pton()");
    exit(EXIT_FAILURE);
  }
  // alternative way to get the ip address back
  // Resolve target using getaddrinfo().
  strcpy(trg_ip, argv[2]);
  if ((status = getaddrinfo(trg_ip, NULL, NULL, &res)) != 0)
  {
    perror("getaddrinfo()");
        exit(EXIT_FAILURE);
  }
  struct sockaddr_in *ipv4 = (struct sockaddr_in *)res->ai_addr;
  memcpy(&arp_req->trg_ip, &ipv4->sin_addr, 4 * sizeof(uint8_t));

  //setting fields of of requests
  for (int i = 0; i < MAC_LENGTH; i++)
  {

    //broadcast address
    send_req->h_dest[i] = (unsigned char)0xff;
    //0 this out because we dont know the target
    arp_req->ar_tha[i] = (unsigned char)0x00;
    /* Filling the source  mac address in the header*/
    send_req->h_source[i] = (unsigned char)ifr_ip.ifr_hwaddr.sa_data[i];
    arp_req->ar_sha[i] = (unsigned char)ifr_ip.ifr_hwaddr.sa_data[i];
    socket_address.sll_addr[i] =
        (unsigned char)ifr_ip.ifr_hwaddr.sa_data[i];
  }
  printf(
      "Successfully got interface MAC address: %02X:%02X:%02X:%02X:%02X:%02X\n",
      send_req->h_source[0], send_req->h_source[1], send_req->h_source[2],
      send_req->h_source[3], send_req->h_source[4], send_req->h_source[5]);

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
  if ((sockfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0)
  {
    perror("socket() failed ");
    exit(EXIT_FAILURE);
  }

  //size of a complete arp is 42, seen via arping in linux
  ret = sendto(sockfd, buffer, 42, 0, (struct sockaddr *)&socket_address,
               sizeof(socket_address));
  if (ret == -1)
  {
    perror("sendto():");
    exit(1);
  }

  printf("\n");
  memset(buffer, 0x00, BUF_SIZ);
  while (1)
  {
    length = recvfrom(sockfd, buffer, BUF_SIZ, 0, NULL, NULL);
    if (length == -1)
    {
      perror("recvfrom():");
      exit(1);
    }

    //validating the arp reply
    if (htons(rcv_resp->h_proto) == ETH_P_ARP)
    {
      if (arp_resp->ar_op == ARP_REPLY)
        printf("RECEIVED ARP RESP len=%d \n", length);

      // print this one
      printf("MAC learned from ARP: ");
      for (int i = 0; i < MAC_LENGTH; i++)
      {
        if (i < MAC_LENGTH-1)
        {

          printf("%02X:", arp_resp->ar_sha[i]);
        }
        else
        {
          printf("%02X", arp_resp->ar_sha[i]);
        }
      }

      // check against this one
      printf("\nOur MAC (prints if response dest MAC matches our MAC): ");
      for (int i = 0; i < MAC_LENGTH; i++)
      {
        if ((unsigned char)ifr_ip.ifr_hwaddr.sa_data[i] ==
                arp_resp->ar_tha[i] &&
            i < (MAC_LENGTH-1))
          printf("%02X:", arp_resp->ar_tha[i]);
        else
        {

          printf("%02X", arp_resp->ar_tha[i]);
        }
      }

      printf("\n");
      break;
    }
  }
  return 0;
}