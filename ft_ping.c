#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netinet/ip_icmp.h>
#include <fcntl.h>
#include <resolv.h>

#define PACKETSIZE	64

struct packet
{
	struct icmphdr hdr;
	char msg[PACKETSIZE-sizeof(struct icmphdr)];
};

int pid=-1;

unsigned short checksum(void *b, int len)
{	unsigned short *buf = b;
	unsigned int sum=0;
	unsigned short result;

	for ( sum = 0; len > 1; len -= 2 )
		sum += *buf++;
	if ( len == 1 )
		sum += *(unsigned char*)buf;
	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);
	result = ~sum;
	return result;
}

int		main(int argc,char **argv)
{
	int sock;
	int i, cnt=1;
	struct hostent *host;
	struct hostent *host2;
	struct sockaddr_in sin = {0};
	struct sockaddr_in sin2 = {0};
	struct msghdr *message;
	char buffer[1024];

    if (argv[1])
	{
		const int val=255;
		struct packet pckt;
		sock = socket(AF_INET, SOCK_RAW, IPPROTO_IP);
		host = gethostbyname(argv[1]);

		if (sock == -1)
			perror("socket");
		if (host == NULL)
			printf("host is null\n");
		if (setsockopt(sock, IPPROTO_IP, IP_TTL, &val, sizeof(val)) != 0)
			perror("Set TTL option");
		sin.sin_addr.s_addr =*((unsigned long *)host->h_addr_list[0]);
		sin.sin_port = htons(8888);
		sin.sin_family = AF_INET;

		bzero(&pckt, sizeof(pckt));
		pckt.hdr.type = ICMP_ECHO;
		pckt.hdr.un.echo.id = pid;
		for ( i = 0; i < sizeof(pckt.msg)-1; i++ )
			pckt.msg[i] = i+'0';
		pckt.msg[i] = 0;
		pckt.hdr.un.echo.sequence = cnt++;
		pckt.hdr.checksum = checksum(&pckt, sizeof(pckt));
		if ( sendto(sock, &pckt, sizeof(pckt), 0, (struct sockaddr*)&sin, sizeof(struct sockaddr)) <= 0 )
			perror("sendto");
	 	else
	 	{
	 		printf("send success\n");
	 		struct sockaddr_storage src_addr;
	 		struct msghdr message;
	 		struct iovec iov[1];
			iov[0].iov_base=buffer;
			iov[0].iov_len=sizeof(buffer);
			message.msg_name=&src_addr;
			message.msg_namelen=sizeof(src_addr);
			message.msg_iov=iov;
			message.msg_iovlen=1;
			message.msg_control=0;
			message.msg_controllen=0;
	 		if (recvmsg(sock, &message, MSG_PEEK) == -1)
	 			perror("recvmsg");
	 		else
			{
	 			printf("received message\n");
			}
			printf("str -> %s\n",message.msg_control);
	 	}
	}
    else
    	printf("usage: ping [ip]\n");
	return (0);
}
