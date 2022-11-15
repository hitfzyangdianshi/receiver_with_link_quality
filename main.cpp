#define _CRT_SECURE_NO_WARNINGS

#include <bits/stdc++.h>

#include<sys/types.h>
#include<netinet/in.h>
#include<netdb.h>
#include<sys/socket.h>
#include<sys/wait.h>
#include<arpa/inet.h>

using namespace std;



struct data_packet {
    __uint16_t senderID;
    __uint32_t packet_num;

    __uint64_t tv_sec;		/* Seconds.  */
    __uint64_t tv_usec;	/* Microseconds.  */ // __SLONGWORD_TYPE	long int

    __int32_t payload[10];
}; // sizeof(PACKET) should be 64
typedef struct data_packet PACKET;

struct timeval current_time;

queue <__uint32_t> buffer_storage_packet_num;


void clear_q(queue<__uint32_t>& q) {
    queue<__uint32_t> empty;
    swap(empty, q);
}


int main(int argc, char** argv)
{
    if (argc < 2) {
        cout << "please give the port number for this receiver " << endl;
        exit(EXIT_FAILURE);
    }
    int port = atoi(argv[1]);

    gettimeofday(&current_time, NULL);

    char outputfilename[100], outputfilelinkquality[100];
    // sprintf(outputfilename, "output port %d starttime %ld.txt", port, current_time.tv_sec);
    sprintf(outputfilelinkquality, "output_link_quality port %d starttime %ld.txt", port, current_time.tv_sec);
    // FILE* outfile = fopen(outputfilename, "w");
    FILE* outlinkquality = fopen(outputfilelinkquality, "w");

    
    PACKET packet;

    int sockListen;
    if ((sockListen = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        fprintf(stderr,"socket fail, errno=%d, %s\n", errno, strerror(errno));
        return -1;
    }
    int set = 1;
    setsockopt(sockListen, SOL_SOCKET, SO_REUSEPORT, &set, sizeof(int));
    struct sockaddr_in recvAddr;
    memset(&recvAddr, 0, sizeof(struct sockaddr_in));
    recvAddr.sin_family = AF_INET;
    recvAddr.sin_port = htons(port);
    recvAddr.sin_addr.s_addr = INADDR_ANY;
    if (bind(sockListen, (struct sockaddr*)&recvAddr, sizeof(struct sockaddr)) == -1) {
        fprintf(stderr, "bind fail, errno=%d, %s\n", errno, strerror(errno));
        return -1;
    }

    int recvbytes;
    unsigned int addrLen = sizeof(struct sockaddr_in);
    unsigned long counter = 0;
    while (true) {

        if ((recvbytes = recvfrom(sockListen, &packet, sizeof(packet), 0, (struct sockaddr*)&recvAddr, &addrLen)) != -1) {
            cout << packet.senderID << "\t" << packet.packet_num << "\t" << packet.tv_sec << "\t" << packet.tv_usec << "\t" << packet.payload[0] << endl;
            // fprintf(outfile, "%lu,%u,%u,%lu,%lu\n", counter, packet.senderID, packet.packet_num, packet.tv_sec, packet.tv_usec);
            buffer_storage_packet_num.push(packet.packet_num);
        }
        else {
            fprintf(stderr, "recvfrom fail, errno=%d, %s\n", errno, strerror(errno));
        }

        counter++;
        if (counter % 50 == 0) {
            double lq;
            lq = ((double)(buffer_storage_packet_num.size())) / ((double)(buffer_storage_packet_num.back()- buffer_storage_packet_num.front()+1));
            printf("link quality: %f\n", lq);
            fprintf(outlinkquality, "%lu,%u,%u,%lu,%lu,%f\n", counter-1, packet.senderID, packet.packet_num, packet.tv_sec, packet.tv_usec,lq);
            clear_q(buffer_storage_packet_num);
        }
        fflush(NULL);
    }

    return 0;
}