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
//#include <wchar.h>
#define MAXLINE 1024 /*max text line length*/
//#define MAXLINE 256 /*max text line length*/
#define SERV_PORT 10010  /*port*/

int main(int argc, char **argv) // user specifies server ip address in command line
{
    int sockfd;
	socklen_t servlen;
    struct sockaddr_in servaddr;
    char sendline[MAXLINE], recvline[MAXLINE];
    struct packet_lab11
    {
        uint16_t len;
        uint32_t seq;
        uint64_t timestamp;
        char message[];
        //wchar_t message[MAXLINE];
    } Packet;
    Packet *pkt = mallon(1);

    // basic check of the arguments
    // additional checks can be inserted
    if (argc != 2)
    {
        perror("Usage: client11b <IP address of the server>");
        exit(1);
    }

    // Create a socket for the client
    // If sockfd < 0 there was an error in the creation of the socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("Problem in creating the socket");
        exit(2);
    }

    servlen = sizeof(servaddr);

    /*
    uint16_t
    uint32_t
    uint64_t
    */

    // Creation of the socket
    memset(&servaddr, 0, servlen);
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(argv[1]); // convert number_dot string to binary
    servaddr.sin_port = htons(SERV_PORT); // convert to big-endian order
    
    printf("Enter a message to send: ");
    while (fgets(sendline, MAXLINE, stdin) != NULL)
    {
        /*
        // Creation of the packet
        memset(&pkt, 0, sizeof(pkt));
        pkt.len = htons(1); // message length
        pkt.seq = htonl(1); // sequence number
        */
        if (sendto(sockfd, sendline, MAXLINE, 0, (struct sockaddr *) &servaddr, servlen) < 0)
        {
            perror("Problem in sending to the server");
            exit(3);
        }
        int numBytes = recvfrom(sockfd, recvline, MAXLINE-1, 0, (struct sockaddr *) &servaddr, &servlen);

        printf("%s", "String received from the server: ");
        fputs(recvline, stdout);

        printf("Enter a message to send: ");
    }
    
    exit(0);
}