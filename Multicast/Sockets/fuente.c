/*
** Fichero: fuente.c
** Autores:
** Juan José López Gómez DNI 70926305D
** Sergio Sánchez García DNI 70961594Q
** Usuario: i0926305
*/

#include <sys/types.h>     
#include <sys/socket.h>     
#include <sys/time.h>       
#include <time.h>           
#include <netinet/in.h>     
#include <arpa/inet.h>      
#include <errno.h>
#include <fcntl.h>          
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>       
#include <sys/uio.h>        
#include <unistd.h>
#include <sys/wait.h>
#include <net/if.h>

#define TAM 4096
#define DEFAULT_PORT 6969
#define DEFAULT_MSG "default message to be sent"
#define DEFAULT_IPV6_MULTICAST "ff15::33"
#define DEFAULT_INTERFACE "eth0"
#define DEFAULT_HOPS 15
#define DEFAULT_INTERVAL 1
int register_SIGINT();
void SIGINT_hdlr();

int main (int argc, char **argv){

struct ipv6_mreq ipv6mreq;
struct addrinfo *multAddr;

int sock, interface_index;
char *ip, *interface, *msg, *prompt;
int lenght_prompt, hops, interval, i;
int port, len;

struct sockaddr_in6 addr, multaddr;

if (argc != 7 && argc != 1){
	fprintf(stderr,"The form of use is: %s <msg> <IPv6 Group Multicast> <interface> <port> <hops> <interval> \n", argv[0]);
	exit(-1);
}
else if (argc == 7){
	if (-1 == register_SIGINT()) 
        	return -1;

	prompt = argv[1];
	ip = argv[2];
	interface = argv[3];
	port = atoi(argv[4]);
	hops = atoi(argv[5]);
	interval = atoi(argv[6]);
	lenght_prompt = strlen(prompt);
}else{
	if (-1 == register_SIGINT()) 
        	return -1;

	prompt = DEFAULT_MSG;
	ip = DEFAULT_IPV6_MULTICAST;
	interface = DEFAULT_INTERFACE;
	port = DEFAULT_PORT;
	hops = DEFAULT_HOPS;
	interval = DEFAULT_INTERVAL;
	lenght_prompt = strlen(prompt);
}

printf("Parameters of execution:\nMessage: %s\nIP: %s\nInterface: %s\nPort: %d\nHops: %d\nInterval: %d\n",prompt, ip, interface, port, hops, interval);

// Socket creation 
if((sock = socket(AF_INET6,SOCK_DGRAM,0)) < 0){
	perror("Error in the socket creation");
	return -1;
}

// Erasing data and loading the structs with the IPv6 configuration
bzero(&addr, sizeof(addr));
addr.sin6_family = AF_INET6;
addr.sin6_addr   = in6addr_any;
addr.sin6_port   = htons(port-1);	

bzero(&multaddr, sizeof(multaddr));
multaddr.sin6_family = AF_INET6;
multaddr.sin6_addr   = in6addr_any;
multaddr.sin6_port   = htons(port);

bzero(&ipv6mreq, sizeof(ipv6mreq));

// Converting IPv6 address to binary
if (inet_pton(AF_INET6,ip, &multaddr.sin6_addr) <= 0) {
	perror("inet_pton: error while converting the address \n");
	exit (-2);
}

if (inet_pton(AF_INET6,ip, &ipv6mreq.ipv6mr_multiaddr) == -1) {
	perror("inet_pton2: error while converting the address \n");
	exit (-3);
}



// Setting the interface which data will we sent
ipv6mreq.ipv6mr_interface=if_nametoindex(interface);

// Joining multicast group
interface_index=if_nametoindex(interface);
if ((setsockopt(sock, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP, &ipv6mreq, sizeof(ipv6mreq))) == -1){
		perror("setsockopt: Error while adding the membership to the group\n");
		exit(-7);
	}
if((setsockopt(sock,IPPROTO_IPV6,IPV6_MULTICAST_IF,(char *)&interface_index, sizeof(interface_index))) == -1){
	perror("setsockopt: Error while setting the interface\n");
	exit(-4);
}

if(setsockopt(sock, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, &hops, sizeof(hops)) == -1) {
	perror("setsockopt: Error while setting the hops\n");
	exit(-5);
}

// Infinite loop until Control + C is pressed
while (1){
	if(sendto(sock,prompt,strlen(prompt),0, (struct sockaddr *)&multaddr,sizeof(struct sockaddr_in6))==-1){
		perror("Sendto: Error while sending data\n");
		exit(-6);
	}	
	sleep(interval);
}

}

int register_SIGINT()
{
    int value = -5;
    sigset_t sigset;
    struct sigaction sa_sigint;

	sigfillset(&sigset);
	sigdelset(&sigset, SIGINT);
		
	sa_sigint.sa_handler = SIGINT_hdlr;
	sa_sigint.sa_flags = 0;

	value = sigfillset(&sa_sigint.sa_mask);
    if (-1 == value){
        perror("\n\nregister_SIGINT: error in sigfillset()\n\n");
        return -1;
    }

	value = sigaction(SIGINT, &sa_sigint, NULL);
    if (-1 == value){
        perror("\n\nregister_SIGINT: error in sigaction()\n\n");
        return -1;
    }

    return 0;
}

void SIGINT_hdlr()
{
 printf("\n\nTerminated\n\n");
 exit(1);
}

