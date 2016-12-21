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
	int sock;
	char buffer[1024];
	const int val=255;
	struct cmsghdr *cmsg;
	int ret;
	struct sock_extended_err *sock_err;

	sock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
	if (sock == -1)
		perror("socket");
	if (setsockopt(sock, IPPROTO_IP, IP_TTL, &val, sizeof(val)) != 0)
		perror("Set TTL option");
	while (42)
	{
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
		ret = recvmsg(sock, &message, MSG_ERRQUEUE);
	    if (ret < 0)
	        continue;
	    /* Control messages are always accessed via some macros 
	     * http://www.kernel.org/doc/man-pages/online/pages/man3/cmsg.3.html
	     */
	    for (cmsg = CMSG_FIRSTHDR(&message);cmsg; cmsg = CMSG_NXTHDR(&message, cmsg)) 
	    {
            /* We received an error */
            if (cmsg->cmsg_type == IP_RECVERR)
            {
                printf("We got IP_RECVERR message\n");
                int *ttlptr = (int *) CMSG_DATA(cmsg);
               	int received_ttl = *ttlptr;
               	printf("ttl -> %d\n", received_ttl);
            }
	    }
	}
}

void	ping(char *ip)
{
	int sock;
	int i, cnt=1;
	struct hostent *host;
	struct hostent *host2;
	struct sockaddr_in sin = {0};
	struct sockaddr_in sin2 = {0};
	struct msghdr *message;
	char buffer[1024];
	const int val=255;
	struct packet pckt;

	sock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
	host = gethostbyname(ip);

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
	if (sendto(sock, &pckt, sizeof(pckt), 0, (struct sockaddr*)&sin, sizeof(struct sockaddr)) <= 0 )
		perror("sendto");
 	else
 	{
 		printf("send success\n");
 	}
}

int		main(int argc,char **argv)	
{

    if (argv[1])
	{
		if (fork() == 0)
			listener();
		else
			ping(argv[1]);
	}
    else
    	printf("usage: ping [ip]\n");
	return (0);
}
