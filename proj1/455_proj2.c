#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/ether.h>
#include <linux/ip.h>
#include <linux/tcp.h>

#define BUF_SIZ		65536
#define SEND 0
#define RECV 1

char interfaceName[IFNAMSIZ];
char buf[BUF_SIZ];
char hw_addr[6];

void send_message(){
	int sockfd, i;
	struct ifreq if_idx;
	struct ifreq if_mac;
	int tx_len = 0;
	char sendbuf[BUF_SIZ];
	struct ether_header *eh = (struct ether_header *) sendbuf;
	struct iphdr *iph = (struct iphdr *) (sendbuf + sizeof(struct ether_header));
	struct sockaddr_ll socket_address;

	//open socket for sending
	if ((sockfd = socket(AF_PACKET, SOCK_RAW, ETH_P_ALL)) == -1) {
	    perror("socket");
	}

	/* Get the index of the interface to send on */
	memset(&if_idx, 0, sizeof(struct ifreq));
	strncpy(if_idx.ifr_name, interfaceName, IFNAMSIZ-1);
	if (ioctl(sockfd, SIOCGIFINDEX, &if_idx) < 0)
	    perror("SIOCGIFINDEX");


	/* Get the MAC address of the interface to send on */
	memset(&if_mac, 0, sizeof(struct ifreq));
	strncpy(if_mac.ifr_name, interfaceName, IFNAMSIZ-1);
	if (ioctl(sockfd, SIOCGIFHWADDR, &if_mac)==0) {
		printf("Sending on mac address: ");	
  	  	for (int i = 0; i < 6; ++i)
		{
			printf("%02x", (unsigned char) if_mac.ifr_addr.sa_data[i]);
			//setup source mac addy while printing
			eh->ether_shost[i] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[i];
			if(i<5)
				printf(":");

		}
		printf("\n");

		//print dest addy as well as set up more of ether packet
		printf("Sending to mac address: ");	
		for(int i = 0; i<6;++i)
		{
			printf("%02x", (unsigned char) hw_addr[i]);
			//setup dest hardware addy while we're at it
			eh->ether_dhost[i] = (uint8_t)hw_addr[i];
			socket_address.sll_addr[i] = (uint8_t)hw_addr[i];

			//pretty print
			if(i<5)
				printf(":");
		}
		printf("\n");
	}
	else
	{
	    perror("SIOCGIFHWADDR");
	}

	/* Ethertype field */
	eh->ether_type = htons(ETH_P_IP);
	tx_len += sizeof(struct ether_header);


	// printf("strlen %d\n",strlen(buf));
	//copy string into buffer to send
	for(int i = 0; i<strlen(buf);i++)
	{
		sendbuf[tx_len++] = buf[i];
	}	

	/* Index of the network device */
	socket_address.sll_ifindex = if_idx.ifr_ifindex;
	/* Address length*/
	socket_address.sll_halen = ETH_ALEN;

	/* Send packet */
	if (sendto(sockfd, sendbuf, tx_len, 0, (struct sockaddr*)&socket_address, sizeof(struct sockaddr_ll)) < 0)
	    printf("Send failed\n");

}

void recv_message(){
	char sender[INET6_ADDRSTRLEN];
	int sockfd, ret, i;
	int sockopt;
	ssize_t numbytes;
	struct ifreq ifopts;	/* set promiscuous mode */
	struct ifreq if_ip;	/* get ip addr */
	struct ifreq if_mac;
	struct sockaddr_storage their_addr;
	uint8_t buf[BUF_SIZ];
	

	/* Header structures */
	struct ether_header *eh = (struct ether_header *) buf;
	struct iphdr *iph = (struct iphdr *) (buf + sizeof(struct ether_header));
	struct udphdr *udph = (struct udphdr *) (buf + sizeof(struct iphdr) + sizeof(struct ether_header));

	memset(&if_ip, 0, sizeof(struct ifreq));

	/* Open PF_PACKET socket, listening for EtherType ETHER_TYPE */
	if ((sockfd = socket(PF_PACKET, SOCK_RAW, htons(ETHER_TYPE))) == -1) {
		perror("listener: socket");	
		return -1;
	}

	//dynamically set recieving mac
	memset(&if_mac, 0, sizeof(struct ifreq));
	strncpy(if_mac.ifr_name, interfaceName , IFNAMSIZ-1);
	if (ioctl(sockfd, SIOCGIFHWADDR, &if_mac)==0) {
		printf("Sending on mac address: ");	
  	  	for (int i = 0; i < 6; ++i)
		{
			printf("%02x", (unsigned char) if_mac.ifr_addr.sa_data[i]);
			//setup source mac addy while printing
			if(i<5)
				printf(":");

		}
		printf("\n");
	}


	/* Set interface to promiscuous mode - do we need to do this every time? */
	strncpy(ifopts.ifr_name, interfaceName, IFNAMSIZ-1);
	ioctl(sockfd, SIOCGIFFLAGS, &ifopts);
	ifopts.ifr_flags |= IFF_PROMISC;
	ioctl(sockfd, SIOCSIFFLAGS, &ifopts);
	/* Allow the socket to be reused - incase connection is closed prematurely */
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof sockopt) == -1) {
		perror("setsockopt");
		close(sockfd);
		exit(EXIT_FAILURE);
	}
	/* Bind to device */
	if (setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, interfaceName, IFNAMSIZ-1) == -1)	{
		perror("SO_BINDTODEVICE");
		close(sockfd);
		exit(EXIT_FAILURE);
	}

	printf("listener: Waiting to recvfrom...\n");
	numbytes = recvfrom(sockfd, buf, BUF_SIZ, 0, NULL, NULL);
	while (numbytes<1)
	{
		//wait to recieve bytes
	}
	
	printf("listener: got packet %lu bytes\n", numbytes);

	/* Check the packet is for me */
	if (eh->ether_dhost[0] == (unsigned char)if_mac.ifr_addr.sa_data[0] &&
			eh->ether_dhost[1] == (unsigned char) if_mac.ifr_addr.sa_data[1] &&
			eh->ether_dhost[2] == (unsigned char) if_mac.ifr_addr.sa_data[2] &&
			eh->ether_dhost[3] == (unsigned char) if_mac.ifr_addr.sa_data[3] &&
			eh->ether_dhost[4] ==  (unsigned char) if_mac.ifr_addr.sa_data[4] &&
			eh->ether_dhost[5] == (unsigned char) if_mac.ifr_addr.sa_data[5] ){
		printf("Correct destination MAC address\n");
	} 

	/* UDP payload length */
	ret = ntohs(udph->len) - sizeof(struct udphdr);

	/* Print packet */
	printf("Data:	");
	for (i=12; i<numbytes; i++) 
		printf("%c", buf[i]);
	printf("\n");

	close(sockfd);
}

int main(int argc, char *argv[])
{
	int mode;

	memset(buf, 0, BUF_SIZ);
	
	int correct=0;
	if (argc > 1){
		if(strncmp(argv[1],"Send", 4)==0){
			if (argc == 5){
				//set program mode to send, parse mac, store message
				mode=SEND; 
				sscanf(argv[3], "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &hw_addr[0], &hw_addr[1], &hw_addr[2], &hw_addr[3], &hw_addr[4], &hw_addr[5]);
				strncpy(buf, argv[4], BUF_SIZ);
				correct=1;
				printf("  buf: %s\n", buf);

			}
		}
		else if(strncmp(argv[1],"Recv", 4)==0){
			if (argc == 3){
				mode=RECV;
				correct=1;
			}

		}
		// IFNAMESIZ defined in net/if.h so ignore lint
		strncpy(interfaceName, argv[2], IFNAMSIZ);
	 }
	 
	 //console notice for how to run
	 if(!correct){
		fprintf(stderr, "./455_proj2 Send <InterfaceName>  <DestHWAddr> <Message>\n");
		fprintf(stderr, "./455_proj2 Recv <InterfaceName>\n");
		exit(1);
	 }

	//Do something here

	if(mode == SEND){
		send_message();
	}
	else if (mode == RECV){
		recv_message();
	}

	return 0;
}

