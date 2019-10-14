#include <stdio.h>
#include <stdlib.h>
#include <string.h>           // strcpy, memset(), and memcpy()
  
#include <netdb.h>            // struct addrinfo
#include <sys/socket.h>       // needed for socket()
#include <netinet/in.h>       // IPPROTO_RAW, INET_ADDRSTRLEN
#include <netinet/ip.h>       // IP_MAXPACKET (which is 65535)
#include <arpa/inet.h>        // inet_pton() and inet_ntop()
#include <sys/ioctl.h>        // macro ioctl is defined
#include <net/if.h>           // struct ifreq
#include <linux/if_ether.h>   // ETH_P_ARP = 0x0806
#include <linux/if_packet.h>  // struct sockaddr_ll (see man 7 packet)
#include <netinet/ether.h>
  
// Define a struct for ARP header
typedef struct _arp_hdr arp_hdr;
struct _arp_hdr {
  uint16_t ar_hrd;
  uint16_t ar_pro;
  unsigned char ar_hln;
  unsigned char ar_pln;
  uint16_t ar_op;
  unsigned char ar_sha[6];
  unsigned char ar_sip[4];
  unsigned char ar_tha[6];
  unsigned char ar_tip[4];
};
  
// Define some constants.
#define ETH_HDRLEN 14      // Ethernet header length
#define IP4_HDRLEN 20      // IPv4 header length
#define ARP_HDRLEN 28      // ARP header length
#define ARPOP_REQUEST 1    // Taken from <linux/if_arp.h>
  
  
int main (int argc, char **argv)
{
  int i, status, frame_length, sd, bytes;
  // char *interface, *target, *src_ip;
  arp_hdr arphdr;
  // uint8_t *src_mac, *dst_mac, *ether_frame;
  struct addrinfo *res;
  struct sockaddr_in *ipv4;
  struct sockaddr_ll device;
  struct ifreq ifr, if_idx;

  unsigned char src_mac[6];
  unsigned char dst_mac[6];
  unsigned char ether_frame[IP_MAXPACKET];
  char interface[40];
  char target[40];
  char src_ip[16];
  
  
  // Interface to send packet through.
  strcpy (interface, argv[1]);
  
  // Submit request for a socket descriptor to look up interface.
  if ((sd = socket (AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0) {
    perror ("socket() failed to get socket descriptor for using ioctl() ");
    exit (EXIT_FAILURE);
  }
  
  // Use ioctl() to look up interface name and get its MAC address.
  memset (&ifr, 0, sizeof (ifr));
  snprintf (ifr.ifr_name, sizeof (ifr.ifr_name), "%s", interface);
  if (ioctl (sd, SIOCGIFHWADDR, &ifr) < 0) {
    perror ("ioctl() failed to get source MAC address ");
    return (EXIT_FAILURE);
  }
  close (sd);
  
  // Copy source MAC address.
  memcpy (src_mac, ifr.ifr_hwaddr.sa_data, 6 * sizeof (uint8_t));
  
  // Report source MAC address to stdout.
  printf ("MAC address for interface %s is ", interface);
  for (i=0; i<5; i++) {
    printf ("%02x:", src_mac[i]);
  }
  printf ("%02x\n", src_mac[5]);
  
  // Find interface index from interface name and store index in
  // struct sockaddr_ll device, which will be used as an argument of sendto().
  memset (&device, 0, sizeof (device));
  if ((device.sll_ifindex = if_nametoindex (interface)) == 0) {
    perror ("if_nametoindex() failed to obtain interface index ");
    exit (EXIT_FAILURE);
  }
  printf ("Index for interface %s is %i\n", interface, device.sll_ifindex);
  
  // Set destination MAC address: broadcast address
  memset (dst_mac, 0xff, 6 * sizeof (uint8_t));
  
  sd = socket(AF_INET,SOCK_DGRAM,0);
  memset(&if_idx, 0, sizeof(struct ifreq));
  strncpy(if_idx.ifr_name, argv[1], IF_NAMESIZE - 1);
  if (ioctl(sd, SIOCGIFADDR, &if_idx) < 0)
    perror("SIOCGIFADDR");
  close(sd);
  strcpy(src_ip, inet_ntoa(((struct sockaddr_in*)&if_idx.ifr_addr)->sin_addr));
  
  //target ip address
  strcpy (target, argv[2]);
  
   // Source IP address
  if ((status = inet_pton (AF_INET, src_ip, &arphdr.ar_sip)) != 1) {
    fprintf (stderr, "inet_pton() failed for source IP address.\nError message: %s", strerror (status));
    exit (EXIT_FAILURE);
  }
  
  //alternative way to get the ip address back
  // Resolve target using getaddrinfo().
  if ((status = getaddrinfo (target, NULL, NULL, &res)) != 0) {
    fprintf (stderr, "getaddrinfo() failed\n");
    exit (EXIT_FAILURE);
  }
  
  //assign address then free
  ipv4 = (struct sockaddr_in *) res->ai_addr;
  memcpy (&arphdr.ar_tip, &ipv4->sin_addr, 4 * sizeof (uint8_t));
  freeaddrinfo (res);
  
  // Fill out sockaddr_ll.
  device.sll_family = AF_PACKET;
  memcpy (device.sll_addr, src_mac, 6 * sizeof (uint8_t));
  device.sll_halen = 6;
  
  // ARP header
  
  // Hardware type (16 bits): 1 for ethernet
  arphdr.ar_hrd = htons (1);
  
  // Protocol type (16 bits): 2048 for IP
  arphdr.ar_pro = htons (ETH_P_IP);
  
  // Hardware address length (8 bits): 6 bytes for MAC address
  arphdr.ar_hln = 6;
  
  // Protocol address length (8 bits): 4 bytes for IPv4 address
  arphdr.ar_pln = 4;
  
  // ar_op: 1 for ARP request
  arphdr.ar_op = htons (ARPOP_REQUEST);
  
  // Sender hardware address (48 bits): MAC address
  memcpy (&arphdr.ar_sha, src_mac, 6 * sizeof (uint8_t));
  
  // Sender protocol address (32 bits)
  // See getaddrinfo() resolution of src_ip.
  
  // Target hardware address (48 bits): zero, since we don't know it yet.
  memset (&arphdr.ar_tha, 0, 6 * sizeof (uint8_t));
  
  // Target protocol address (32 bits)
  // See getaddrinfo() resolution of target.
  
  // Fill out ethernet frame header.
  
  // Ethernet frame length = ethernet header (MAC + MAC + ethernet type) + ethernet data (ARP header)
  frame_length = 6 + 6 + 2 + ARP_HDRLEN;
  
  // Destination and Source MAC addresses
  memcpy (ether_frame, dst_mac, 6 * sizeof (uint8_t));
  memcpy (ether_frame + 6, src_mac, 6 * sizeof (uint8_t));
  
  // Next is ethernet type code (ETH_P_ARP for ARP).
  // http://www.iana.org/assignments/ethernet-numbers
  ether_frame[12] = ETH_P_ARP / 256;
  ether_frame[13] = ETH_P_ARP % 256;
  
  // Next is ethernet frame data (ARP header).
  
  // ARP header
  memcpy (ether_frame + ETH_HDRLEN, &arphdr, ARP_HDRLEN * sizeof (uint8_t));
  
  // Submit request for a raw socket descriptor.
  if ((sd = socket (PF_PACKET, SOCK_RAW, htons (ETH_P_ALL))) < 0) {
    perror ("socket() failed ");
    exit (EXIT_FAILURE);
  }
  
  // Send ethernet frame to socket.
  if ((bytes = sendto (sd, ether_frame, frame_length, 0, (struct sockaddr *) &device, sizeof (device))) <= 0) {
    perror ("sendto() failed");
    exit (EXIT_FAILURE);
  }
  
  // Close socket descriptor.
  close (sd);
  
  return (EXIT_SUCCESS);
}
  
