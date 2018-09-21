/*
 * echoserver.c
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
#define MAXLINE 4096 /*max text line length*/
#define SERV_PORT 10010  /*port*/
#define LISTENQ 8 /*maximum number of client connections*/

int main(int argc, char **argv)
{
	int listenfd, connfd, n;
	pid_t childpid;
	socklen_t clilen;
	char buf[MAXLINE];
	struct sockaddr_in cliaddr, servaddr;
    typedef struct __attribute__((__packed__)) request_packet
    {
        char op;
        uint32_t a;
        uint32_t b;
    } Packet_req;
    typedef struct __attribute__((__packed__)) response_packet
    {
        char op;
        uint32_t a;
        uint32_t b;
        uint32_t r;
        char v;
    } Packet_rsp;
    Packet_req req;
    Packet_rsp response;

	//Create a socket for the soclet
	//If sockfd<0 there was an error in the creation of the socket
	if ((listenfd = socket (AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("Problem in creating the socket");
		exit(2);
	}

	//preparation of the socket address
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(SERV_PORT);

	//bind the socket
	bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

	//listen to the socket by creating a connection queue, then wait for clients
	listen(listenfd, LISTENQ);

	printf("%s\n", "Server running... waiting for connections. ");

	for( ; ; )
	{
		clilen = sizeof(cliaddr);
		//accept a connection
		connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &clilen); //blocked, wait for connection

		printf("%s\n", "Received request...");

		if( (childpid = fork()) == 0) // if it's 0, it's child process
		{
			printf("%s\n", "Child created for dealing with client requests");
            memset(&req, 0, sizeof(req));

			//close listening socket
			close (listenfd);

			if((n = recv(connfd, &req, sizeof(req), 0)) >= 0)
			{
                memset(&response, 0, sizeof(response));

                response.op = req.op;
                response.a = req.a;
                response.b = req.b;

				req.a = ntohl(req.a);
				req.b = ntohl(req.b);
				
				printf("%s %c %u %u\n", "Request from a client:", req.op, req.a, req.b);

                if(req.op == '+')
                {
                    response.r = req.a + req.b;
                    response.v = 1;
                }
                else if(req.op == '-')
                {
                    response.r = req.a - req.b;
                    response.v = 1;
                }
                else if(req.op == '*')
                {
                    response.r = req.a * req.b;
                    response.v = 1;
                }
                else if(req.op == '/' && req.b != 0)
                {
                    response.r = (int)req.a / (int)req.b;
                    response.v = 1;
                }
                else // Division by 0 or invalid op code
                {
                    response.r = 0;
                    response.v = 2;
                }

				response.r = htonl(response.r);
				send(connfd, &response, sizeof(response), 0);
			}

			if (n < 0)
			{
				printf("%s\n", "Read error");
			}
			exit(0); // child exit
		}

		//close socket of the server
		close(connfd);
	}

	return 0;
}
