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

void send_message(){

	//Do something here
	// struct ifreq ifreq_i;
	// memset(&ifreq_i,0,sizeof(ifreq_i));
	// strncpy(ifreq_i.ifr_name,wlan0,IFNAMSIZ-1); //giving name of Interface
	
	// if((ioctl(sock_raw,SIOCGIFINDEX,&ifreq_i))<0)
	// printf(error in index ioctl reading);//getting Index Name
	
	// printf(index=%d\n,ifreq_i.ifr_ifindex);	


}

void recv_message(){

	//setting up for recieve
	int sock_r;
	int ipHeaderLen, ethHeaderSize, udpHeaderSize;
	
	//to use IP packets intsead use ETH_P_IP?
	sock_r = socket(AF_PACKET,SOCK_RAW,htons(ETH_P_ALL)); // recieve on all 
	if(sock_r<0)
	{
		printf("socket failed\n");
		return;
	}

	unsigned char *buff = (unsigned char*) malloc(BUF_SIZ);
	memset(buff,0,BUF_SIZ);
	struct sockaddr saddr;
	int saddr_len = sizeof(saddr);

	//now rec packet, write to buff
	int bufferLen = recvfrom(sock_r,buff,BUF_SIZ,0,&saddr,(socklen_t*)&saddr_len);

	printf("waiting to recieve packet...\n");
	while (buff<=0)
	{

	}
	printf("Message Received\n");
	
	//store ethernet header
	struct ethhdr *ether = (struct ethhdr *) (buff);
	ethHeaderSize = sizeof(struct ethhdr);
	//store IP header, and grab the length
	struct iphdr *ip = (struct iphdr *)(buff + ethHeaderSize);
	ipHeaderLen = ip->ihl * 4;
	
	struct tcphdr *tcpHeader = (struct tcphdr*)(buff+ipHeaderLen+sizeof(struct ethhdr));

	unsigned char* user_data = (unsigned char *)((unsigned char *)tcpHeader + (tcpHeader->doff * 4));
	
	printf("%s\n",user_data);

}

int main(int argc, char *argv[])
{
	int mode;
	char hw_addr[6];

	// IFNAMESIZ defined in net/if.h so ignore lint
	char interfaceName[IFNAMSIZ];
	char buf[BUF_SIZ];
	memset(buf, 0, BUF_SIZ);
	
	int correct=0;
	if (argc > 1){
		if(strncmp(argv[1],"Send", 4)==0){
			if (argc == 5){
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
		strncpy(interfaceName, argv[2], IFNAMSIZ);
	 }
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
		while (1)
		{
			recv_message();
		}
	}

	return 0;
}

