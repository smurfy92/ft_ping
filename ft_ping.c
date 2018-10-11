#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
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

void		listener(void)
{
 	struct msghdr *message;
	int ret;
	int sock;
	struct protoent *proto;

	proto = getprotobyname("ICMP");
	sock = socket(AF_INET, SOCK_RAW, proto->p_proto);

 	while (42)
 	{
 		ret = recvmsg(sock, message, 0);
		if (ret > 0)
			printf("received message");
	}
}

void	ping(char *ip)
{
	int sock;
	struct protoent *proto;

	proto = getprotobyname("ICMP");
	sock = socket(AF_INET, SOCK_RAW, proto->p_proto);
}

int		main(int argc,char **argv)
{
	int fd;
	if (argv[1])
	{
		fd = fork();
		if (fd == 0)
			listener(sock);
		else
			ping(argv[1], sock);
	}
	else
    		printf("usage: ping [ip]\n");
	return (0);
}
