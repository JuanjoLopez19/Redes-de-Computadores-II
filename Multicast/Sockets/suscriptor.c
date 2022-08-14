/*
** Fichero: suscriptor.c
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
#define TIMEOUT 6
#define DEFAULT_PORT 6969
#define DEFAULT_IPV6_MULTICAST "ff15::33"
#define DEFAULT_INTERFACE "eth0"

int register_SIGINT();
void SIGINT_hdlr();

int sock;

int main(int argc, char **argv){
int interface_index, len;
char *ip, *interface;
int port, confirm, flag;
char prompt[TAM];
char sender[INET6_ADDRSTRLEN];

struct ipv6_mreq ipv6mreq;
struct sockaddr_in6 addr;
	
if (argc != 1 && argc != 4){

	fprintf(stderr,"The form of use is: %s <IPv6 Group Multicast> <interface> <port>\n", argv[0]);
	exit(-1);

}else if (argc == 4){

	if (-1 == register_SIGINT()) 
        	return -1;

	ip = argv[1];
	interface = argv[2];
	port = atoi(argv[3]);
}else{

	if (-1 == register_SIGINT()) 
        	return -1;

	ip = DEFAULT_IPV6_MULTICAST;
	interface = DEFAULT_INTERFACE;
	port = DEFAULT_PORT;
}

printf("Parameters of execution:\nIP: %s\nInterface: %s\nPort: %d\n",ip, interface, port);

// Socket creation 
if((sock = socket(AF_INET6,SOCK_DGRAM,0)) < 0){
	perror("Error in the socket creation");
	return -1;
}

confirm = 1;
if (setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,(char*)&(confirm),sizeof(int)) == -1){
  	 perror("setsockopt");
}

// Erasing data and loading the structs with the IPv6 configuration
bzero(&addr, sizeof(addr));
addr.sin6_family = AF_INET6;
addr.sin6_flowinfo   = 0;
addr.sin6_port   = htons(port);	


// Converting IPv6 address to binary
if (inet_pton(AF_INET6,ip, &addr.sin6_addr) <= 0) {
	perror("inet_pton: error while converting the address \n");
	exit (-2);
}

if (bind(sock, (struct sockaddr*) &addr,sizeof(struct sockaddr_in6)) < 0) {
	perror ("Error en bind\n");
	exit (-3);
}

ipv6mreq.ipv6mr_interface=if_nametoindex(interface);

// Converting IPv6 address to binary
if(inet_pton(AF_INET6,ip,&ipv6mreq.ipv6mr_multiaddr)==-1){
	perror("inet_pton: error while converting the address \n");
	exit(-4);
}

// Join the multicast group
interface_index=if_nametoindex(interface);
if ((setsockopt(sock, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP, &ipv6mreq, sizeof(ipv6mreq))) == -1){
		perror("setsockopt: Error while adding the membership to the group\n");
		exit(-5);
}

len = sizeof(struct sockaddr_in6);
while (1){
	if((flag=recvfrom(sock, prompt, TAM - 1, 0, (struct sockaddr *) &addr, &len)) == -1){
		perror("recvfrom: Error in the receive \n");
            close(sock);
            exit (-6);
    }else{
       	prompt[flag]='\0';
		if (inet_ntop (AF_INET6, &(addr.sin6_addr), sender, sizeof (sender)) == NULL) {
            perror("inet_ntop: Error on showing the address \n");                      
        }else{
	        printf("%s recibido desde %s\n",prompt ,sender);
	   	}
	    sleep(1);
    }	
}
	close(sock);
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
        perror("\n\nsigfillset: error while filling the mask\n\n");
        return -1;
    	}

	value = sigaction(SIGINT, &sa_sigint, NULL);
    	if (-1 == value){
        perror("\n\nsigaction: error while the register SIGINT\n\n");
        return -1;
    	}
    	
    return 0;
}

void SIGINT_hdlr()
{
 close(sock);
 printf("\n\nTerminated\n\n");
 exit(1);
}


