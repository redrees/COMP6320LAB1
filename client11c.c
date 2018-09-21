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
#include <time.h>
#include <arpa/inet.h>
#include <unistd.h>
#define MAXLINE 1024 /*max text line length*/
#define MAXSIZE MAXLINE+14 /*max packet bytes*/
#define SERV_PORT 10010  /*port*/
#define NUMPACKETS 10000 /*number of packets to send/receive*/

int main(int argc, char **argv) // user specifies server ip address in command line
{
    uint64_t htonll(uint64_t);
    uint64_t ntohll(uint64_t);

    int sockfd;
	socklen_t servlen;
	uint64_t minstime=99999;
	uint64_t maxstime=0;
	uint64_t totaltime=0;
	int messagemissed=0;
    struct sockaddr_in servaddr;
    uint32_t seq = 1;
    struct timespec ts;
    struct timespec timeout;
    timeout.tv_sec = 10;
    timeout.tv_nsec = 0;
    typedef struct __attribute__((__packed__)) packet_lab11
    {
        uint16_t len;
        uint32_t seq;
        uint64_t timestamp;
        char message[MAXLINE];
    } Packet;
    pid_t childpid;

    // basic check of the arguments
    // additional checks can be inserted
    if (argc != 2)
    {
        perror("Usage: client11c <IP address of the server>");
        exit(1);
    }

    // Create a socket for the client
    // If sockfd < 0 there was an error in the creation of the socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("Problem in creating the socket");
        exit(2);
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
    {
        perror("Problem in setting receive timeout");
        exit(3);
    }

    servlen = sizeof(servaddr);

    // Creation of the socket
    memset(&servaddr, 0, servlen);
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(argv[1]); // convert number_dot string to binary
    servaddr.sin_port = htons(SERV_PORT); // convert to big-endian order

    childpid = fork();
    if (childpid == 0) // child process
    {
        // send here
        int i = 0;
        Packet pkt_s;
        while (i++ < NUMPACKETS)
        {
            // Creation of the packet
            memset(&(pkt_s), 0, MAXSIZE);
            sprintf(pkt_s.message, "%d", i);
            int messagelen = (int)strlen(pkt_s.message);
            pkt_s.len = htons(messagelen); // message length
            pkt_s.seq = htonl(seq++); // sequence number

            clock_gettime(CLOCK_REALTIME, &ts);
            pkt_s.timestamp = ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
            pkt_s.timestamp = htonll(pkt_s.timestamp);

            if (sendto(sockfd, &pkt_s, messagelen + 16, 0, (struct sockaddr *) &servaddr, servlen) < 0)
            {
                perror("Problem in sending to the server");
                exit(4);
            }
        }
    }
    else // parent process
    {				//I think we forget to calculate the smallest, largest, and average round trip times in lab11b. 
        // receive here
        int i = 0;
        int j;
        int numBytes = 0;	
        Packet pkt_r;
        int returnedMessages[NUMPACKETS] = { 0 };

        while (i++ < NUMPACKETS && numBytes >= 0)
        {
            memset(&pkt_r, 0, MAXSIZE);
            numBytes = recvfrom(sockfd, &pkt_r, MAXSIZE, 0, (struct sockaddr *) &servaddr, &servlen);
            if(numBytes < 0)
            {
                printf("server timedout?\n");
            }
            else
            {
                returnedMessages[atoi(pkt_r.message)-1] = 1;
// 		    if(atoi(pkt_r.timestamp)<minstime)
// 		    {
// 			minstime=atoi(pkt_r.timestamp);
// 		    }
// 		    if(atoi(pkt_r.timestamp)>maxstime)
// 		    {
// 			maxstime=atoi(pkt_r.timestamp);
// 		    }
// 		    totaltime=atoi(pkt_r.timestamp)+totaltime;
            }
        }

        // report missing echos from the server
        i = 0;
        while (i < NUMPACKETS)
        {
            if (returnedMessages[i++] == 0)
            {
                printf("%s%d\n", "Missing echo: ", i);
// 		messagemissed++;
            }
        }
// 	printf("%s%d\n", "Missing echo totally: ", messagemissed);
// 	printf("%s%d\n", "Smallest round trip time: ", minstime);
// 	printf("%s%d\n", "Largest round trip time: ", maxstime);
// 	printf("%s%d\n", "average round trip time: ", totaltime/(10000-messagemissed));
    }

    exit(0);
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
