#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/ether.h>

#define BUF_SIZ 65536
#define SEND 0
#define RECV 1

char interfaceName[IFNAMSIZ];
char buf[BUF_SIZ];
char hw_addr[6];

void send_message()
{
	int sockfd;
	struct ifreq if_idx;
	struct ifreq if_mac;
	int sendLength = 0;
	char sendbuf[BUF_SIZ];
	//ethernet header
	struct ether_header *eh = (struct ether_header *)sendbuf;
	struct sockaddr_ll socket_address;

	//open a socket to listen on
	if ((sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0)
	{
		perror("socket() failed");
		exit(1);
	}

	//assigning to socket to an interface
	memset(&if_idx, 0, sizeof(struct ifreq));
	strncpy(if_idx.ifr_name, interfaceName, IFNAMSIZ - 1);
	if (ioctl(sockfd, SIOCGIFINDEX, &if_idx) < 0)
		perror("SIOCGIFINDEX");

	//getting mac address of interface
	memset(&if_mac, 0, sizeof(struct ifreq));
	strncpy(if_mac.ifr_name, interfaceName, IFNAMSIZ - 1);

	//modified to print out mac addresses for convienience/debugging
	if (ioctl(sockfd, SIOCGIFHWADDR, &if_mac) == 0)
	{
		printf("Sending on mac address: ");
		for (int i = 0; i < 6; ++i)
		{
			printf("%02x", (unsigned char)if_mac.ifr_addr.sa_data[i]);
			//setup source mac addy while printing
			eh->ether_shost[i] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[i];
			if (i < 5)
				printf(":");
		}
		printf("\n");

		//print dest addy as well as set up more of ether packet
		printf("Sending to mac address: ");
		for (int i = 0; i < 6; ++i)
		{
			printf("%02x", (unsigned char)hw_addr[i]);
			//setup dest hardware addy while we're at it
			eh->ether_dhost[i] = (uint8_t)hw_addr[i];
			socket_address.sll_addr[i] = (uint8_t)hw_addr[i];

			//pretty print
			if (i < 5)
				printf(":");
		}
		printf("\n");
	}
	else
	{
		perror("SIOCGIFHWADDR");
	}

	//some settings for the ethernet header
	eh->ether_type = htons(ETH_P_IP);
	sendLength += sizeof(struct ether_header);

	//copy string into buffer to send
	for (int i = 0; i < strlen(buf); i++)
	{
		sendbuf[sendLength++] = buf[i];
	}

	/* Index of the network device */
	socket_address.sll_ifindex = if_idx.ifr_ifindex;
	/* Address length*/
	socket_address.sll_halen = ETH_ALEN;

	/* Send packet */
	if (sendto(sockfd, sendbuf, sendLength, 0, (struct sockaddr *)&socket_address, sizeof(struct sockaddr_ll)) < 0)
		printf("Send failed\n");

	//clearing the buffer
	memset(buf, 0, BUF_SIZ);
}

void recv_message()
{
	int sockfd;
	ssize_t numbytes;
	struct ifreq if_mac;

	//ethernet header
	struct ether_header *eh = (struct ether_header *)buf;

	//open a socket to listen on
	if ((sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0)
	{
		perror("socket() failed");
		exit(1);
	}

	//modified to print out mac addresses for convienience/debugging
	memset(&if_mac, 0, sizeof(struct ifreq));
	strncpy(if_mac.ifr_name, interfaceName, IFNAMSIZ - 1);
	if (ioctl(sockfd, SIOCGIFHWADDR, &if_mac) == 0)
	{
		printf("Recieving on mac address: ");
		for (int i = 0; i < 6; ++i)
		{
			printf("%02x", (unsigned char)if_mac.ifr_addr.sa_data[i]);
			//setup source mac addy while printing
			if (i < 5)
				printf(":");
		}
		printf("\n");
	}
	else
	{
		perror("SIOCGIFHWADDR");
	}

	printf("Waiting for packet..\n");
	numbytes = recvfrom(sockfd, buf, BUF_SIZ, 0, NULL, NULL);
	while (numbytes < 1)
	{
		//wait to recieve bytes
	}

	printf("got packet %lu bytes\n", numbytes);

	/* Check the packet is for me */
	if (eh->ether_dhost[0] == (unsigned char)if_mac.ifr_addr.sa_data[0] &&
		eh->ether_dhost[1] == (unsigned char)if_mac.ifr_addr.sa_data[1] &&
		eh->ether_dhost[2] == (unsigned char)if_mac.ifr_addr.sa_data[2] &&
		eh->ether_dhost[3] == (unsigned char)if_mac.ifr_addr.sa_data[3] &&
		eh->ether_dhost[4] == (unsigned char)if_mac.ifr_addr.sa_data[4] &&
		eh->ether_dhost[5] == (unsigned char)if_mac.ifr_addr.sa_data[5])
	{
		printf("packet is for this device\n");
	}

	//print out the data
	printf("Message: %s\n", buf + sizeof(*eh));
	close(sockfd);
	//prevent junk messages on reloop
	memset(buf, 0, BUF_SIZ);
}

int main(int argc, char *argv[])
{
	int mode;

	memset(buf, 0, BUF_SIZ);

	int correct = 0;
	if (argc > 1)
	{
		if (strncmp(argv[1], "Send", 4) == 0)
		{
			if (argc == 5)
			{
				//set program mode to send, parse mac, store message
				mode = SEND;
				sscanf(argv[3], "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &hw_addr[0], &hw_addr[1], &hw_addr[2], &hw_addr[3], &hw_addr[4], &hw_addr[5]);
				strncpy(buf, argv[4], BUF_SIZ);
				correct = 1;
				printf("  buf: %s\n", buf);
			}
		}
		else if (strncmp(argv[1], "Recv", 4) == 0)
		{
			if (argc == 3)
			{
				mode = RECV;
				correct = 1;
			}
		}
		// IFNAMESIZ defined in net/if.h so ignore lint
		strncpy(interfaceName, argv[2], IFNAMSIZ);
	}

	//console notice for how to run
	if (!correct)
	{
		fprintf(stderr, "./455_proj2 Send <InterfaceName>  <DestHWAddr> <Message>\n");
		fprintf(stderr, "./455_proj2 Recv <InterfaceName>\n");
		exit(1);
	}

	//Do something here

	if (mode == SEND)
	{
		send_message();
	}
	else if (mode == RECV)
	{
		//loop so we can keep recieving
		while (1)
			recv_message();
	}

	return 0;
}
