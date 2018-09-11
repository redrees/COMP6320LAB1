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
#define MAXLINE 1024 /*max text line length*/
#define SERV_PORT 10010  /*port*/

int main(int argc, char **argv)
{
	int sockfd, connfd, n;
	pid_t childpid;
	socklen_t clilen;
	char buf[MAXLINE];
	struct sockaddr_in cliaddr, servaddr;

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
		//accept a connection
		connfd = recvfrom(sockfd, buf, MAXLINE-1, 0, (struct sockaddr *) &cliaddr, &clilen); //blocked, wait for connection

        if (connfd < 0)
        {
            perror("recvfrom error");
            exit(3);
        }

		printf("%s\n", "String received from and resent to the client:");
        puts(buf);
        sendto(sockfd, buf, MAXLINE, 0, (struct sockaddr *) &cliaddr, clilen);
	}

	//close socket of the server
	close(connfd);

	return 0;
}