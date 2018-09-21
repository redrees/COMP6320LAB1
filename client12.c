/*
 * echoclient.c
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
#include <arpa/inet.h>
#define MAXLINE 4096 /*max text line length*/
#define SERV_PORT 10010  /*port*/

int main(int argc, char **argv) // user specifies server ip address in command line
{
    int sockfd;
    struct sockaddr_in servaddr;
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

    // basic check of the arguments
    // additional checks can be inserted
    if (argc != 5)
    {
        perror("Usage: client12 <IP address of the server> <arithmetic op> <integer> <integer>");
        exit(1);
    }

    // Create a socket for the client
    // If sockfd < 0 there was an error in the creation of the socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Problem in creating the socket");
        exit(2);
    }

    // Creation of the socket
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(argv[1]); // convert number_dot string to binary
    servaddr.sin_port = htons(SERV_PORT); // convert to big-endian order

    // Connection of the client to the socket
    if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
    {
        perror("Problem in connecting to the server");
        exit(3);
    }

    // Creation of the request packet
    memset(&req, 0, sizeof(req));
    req.op = argv[2][0];
    req.a = atoi(argv[3]);
    req.b = atoi(argv[4]);

    if(req.op != '+' && req.op != '-' && req.op != '*' && req.op != '/')
    {
        perror("Invalid op code: must be one of +, -, *, /");
        exit(4);
    }

    printf("%s%c %d %d\n", "request: ", req.op, req.a, req.b);

    req.a = htonl(req.a);
    req.b = htonl(req.b);

    send(sockfd, &req, sizeof(req), 0); // send request to server
    
    memset(&response, 0, sizeof(response));
    if (recv(sockfd, &response, sizeof(response), 0) == 0) // waiting to receive a response from server
    {
        // error: server terminated prematurely
        perror("The server terminated prematurely");
        exit(5);
    }

    if (response.v == 1)
    {
        printf("%s %d\n", "Answer received from the server: ", ntohl(response.r));
    }
    else
    {
        printf("NAN\n");
    }

    exit(0);
}
