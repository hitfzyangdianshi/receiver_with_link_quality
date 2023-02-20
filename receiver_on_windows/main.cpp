#define _CRT_SECURE_NO_WARNINGS

#include <cstdio>
#include <iostream>
#include <chrono>
#include <cinttypes>
#include <queue>

#include<sys/types.h>

#include<winsock2.h>

#pragma comment(lib,"ws2_32.lib") //Winsock Library

using namespace std;
using namespace chrono;

#define __uint16_t uint16_t 
#define __uint32_t uint32_t 
#define __uint64_t uint64_t
#define __int32_t int32_t 
#define __int64_t int64_t 

#define QUEUE_BUFFER_SIZE 50

struct data_packet {
    __uint16_t senderID;
    __uint32_t packet_num;

    __uint64_t time_since_epoch_micro;

    __int32_t payload[1000];
};  
typedef struct data_packet PACKET;

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


    char outputfilename[100], outputfilelinkquality[100], outfiledelay[100];
    // sprintf(outputfilename, "output port %d starttime %ld.txt", port, current_time.tv_sec);

    __uint64_t current_time=duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();

    sprintf(outputfilelinkquality, "output_link_quality port %d.txt", port);
    // FILE* outfile = fopen(outputfilename, "w");
    FILE* outlinkquality = fopen(outputfilelinkquality, "w");
    fprintf(outlinkquality, "counter,senderID,packet_num,microsec,link quality\n");

    sprintf(outfiledelay, "output_delay port %d.txt", port);
    FILE* outdelay = fopen(outfiledelay, "w");
    fprintf(outdelay, "counter,senderID,packet_num,usec_orig,usec_rec,time_diff\n");

    WSADATA wsaDATA;
    WSAStartup(MAKEWORD(2, 2), &wsaDATA);


    PACKET packet;

    SOCKET sockListen;
    if ((sockListen = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        fprintf(stderr,"socket fail, errno=%d, %s\n", errno, strerror(errno));
        return -1;
    }
    // int set = 1;
    // setsockopt(sockListen, SOL_SOCKET, SO_REUSEPORT, &set, sizeof(int));
    struct sockaddr_in recvAddr;
    memset(&recvAddr, 0, sizeof(struct sockaddr_in));
    recvAddr.sin_family = AF_INET;
    recvAddr.sin_port = htons(port);
    recvAddr.sin_addr.s_addr = INADDR_ANY;
    if (bind(sockListen, (SOCKADDR*)&recvAddr, sizeof(struct sockaddr)) == -1) {
        fprintf(stderr, "bind fail, errno=%d, %s\n", errno, strerror(errno));
        return -1;
    }

    int recvbytes;
    int addrLen = sizeof(struct sockaddr_in);
    unsigned long counter = 0;
    double lq;
    while (true) {

        if ((recvbytes = recvfrom(sockListen, (char*)&packet, sizeof(packet), 0, (SOCKADDR*)&recvAddr, &addrLen)) != -1) {
            current_time = duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();
            cout << packet.senderID << "\t" << packet.packet_num << "\t" << packet.time_since_epoch_micro << endl;
            // fprintf(outfile, "%lu,%u,%u,%lu,%lu\n", counter, packet.senderID, packet.packet_num, packet.tv_sec, packet.tv_usec);
            buffer_storage_packet_num.push(packet.packet_num);
            fprintf(outdelay, "%lu,%u,%u,%lu,%lu,%ld\n", counter, packet.senderID, packet.packet_num, packet.time_since_epoch_micro, current_time, (__int64_t)current_time - (__int64_t)packet.time_since_epoch_micro);
        }
        else {
            fprintf(stderr, "recvfrom fail, errno=%d, %s\n", errno, strerror(errno));
        }

        counter++;
       /* if (counter % QUEUE_BUFFER_SIZE == 0) {
            lq = ((double)(buffer_storage_packet_num.size())) / ((double)(buffer_storage_packet_num.back()- buffer_storage_packet_num.front()+1));
            printf("link quality: %f\n", lq);
            fprintf(outlinkquality, "%lu,%u,%u,%lu,%f\n", counter - 1, packet.senderID, packet.packet_num, current_time, lq);
            clear_q(buffer_storage_packet_num);
        }*/

        if (buffer_storage_packet_num.size() >= QUEUE_BUFFER_SIZE) {
            while (buffer_storage_packet_num.size() > QUEUE_BUFFER_SIZE) {
                buffer_storage_packet_num.pop();
            }

            lq = ((double)(buffer_storage_packet_num.size())) / ((double)(buffer_storage_packet_num.back() - buffer_storage_packet_num.front() + 1));
            printf("link quality: %f\n", lq);
            fprintf(outlinkquality, "%lu,%u,%u,%lu,%f\n", counter - 1, packet.senderID, packet.packet_num, current_time, lq);      
        }


        fflush(NULL);
    }

    closesocket(sockListen);
    WSACleanup();

    return 0;
}