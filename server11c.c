/*
 * server11.c
 *
 *  Created on: Sep 9, 2018
 *      Author: Hugh Kwon
 */


#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#define MAXLINE 1024 /*max string bytes*/
#define MAXSIZE MAXLINE+16 /*max packet bytes*/
#define SERV_PORT 10010  /*port*/

int main(int argc, char **argv)
{
    uint64_t htonll(uint64_t);
    uint64_t ntohll(uint64_t);

	int sockfd, numBytes, n;
	pid_t childpid;
	socklen_t clilen;
	char buf[MAXLINE];
	struct sockaddr_in cliaddr, servaddr;
    uint32_t seq = 1;
    typedef struct packet_lab11
    {
        uint16_t len;
        uint32_t seq;
        uint64_t timestamp;
        char message[MAXLINE+1];
    } Packet;
    Packet pkt;
	memset(&pkt, 0, MAXSIZE+1);

	//Create a socket for the soclet
	//If sockfd<0 there was an error in the creation of the socket
	if ((sockfd = socket (AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("Problem in creating the socket");
		exit(2);
	}

	//preparation of the socket address
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(SERV_PORT);

	//bind the socket
	bind(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

	printf("%s\n", "Server running... waiting for connections. ");

	for( ; ; )
	{
		clilen = sizeof(cliaddr);
		//receive data from client
		numBytes = recvfrom(sockfd, &pkt, MAXSIZE, 0, (struct sockaddr *) &cliaddr, &clilen); //blocked, wait for connection

        if (numBytes < 0)
        {
            perror("recvfrom error");
            exit(3);
        }

        //just in case client didn't send a null byte
		pkt.message[numBytes-(MAXSIZE-MAXLINE)] = '\0';

        if(ntohl(pkt.seq)%23 != 0){

		printf("%s\n", "String received from and resent to the client:");
        puts(pkt.message);
		fflush(stdout);
        if(sendto(sockfd, &pkt, MAXSIZE, 0, (struct sockaddr *) &cliaddr, clilen) < 0)
        {
            perror("something is wrong...");
            exit(4);

        } }
	}

	return 0;
}

/*
    Converts 64-bit int to network byte order using htonl
*/
uint64_t htonll(uint64_t value)
{
    if (htonl(1) != 1)
    {
        return (((uint64_t)htonl((uint32_t)(value & 0xFFFFFFFFLL))) << 32) | htonl((uint32_t)(value >> 32));
    }
    else
    {
        return value;
    }
}

/*
    Converts 64-bit int to host byte order using ntohl
*/
uint64_t ntohll(uint64_t value)
{
    if (htonl(1) != 1)
    {
        return (((uint64_t)ntohl((uint32_t)(value & 0xFFFFFFFFLL))) << 32) | ntohl((uint32_t)(value >> 32));
    }
    else
    {
        return value;
    }
}