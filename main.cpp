#define _CRT_SECURE_NO_WARNINGS

#include <bits/stdc++.h>

#include<sys/types.h>
#include<sys/time.h>
#include<netinet/in.h>
#include<netdb.h>
#include<sys/socket.h>
#include<sys/wait.h>
#include<arpa/inet.h>

using namespace std;
using namespace chrono;

#define QUEUE_BUFFER_SIZE 50

struct data_packet {
    __uint16_t senderID;
    __uint32_t packet_num;

    __uint64_t time_since_epoch_micro;

    __int32_t payload[1000];
};  
typedef struct data_packet PACKET;

queue <__uint32_t> buffer_storage_packet_num;
queue <__uint32_t> buffer_storage_packet_num_a;


void clear_q(queue<__uint32_t>& q) {
    queue<__uint32_t> empty;
    swap(empty, q);
}


int main(int argc, char** argv)
{
    if (argc < 3) {
        cout << "please give the port number for this receiver, and the number of packets you want to receive(0=infinite) " << endl;
        exit(EXIT_FAILURE);
    }
    int port = atoi(argv[1]);
    long numbers_of_packets = atoi(argv[2]);


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
    double lq;
    if (numbers_of_packets == 0) {
        while (true) {

            if ((recvbytes = recvfrom(sockListen, &packet, sizeof(packet), 0, (struct sockaddr*)&recvAddr, &addrLen)) != -1) {
                current_time = duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();
                cout << packet.senderID << "\t" << packet.packet_num << "\t" << packet.time_since_epoch_micro << endl;
                // fprintf(outfile, "%lu,%u,%u,%lu,%lu\n", counter, packet.senderID, packet.packet_num, packet.tv_sec, packet.tv_usec);
                if(packet.senderID == port)
                    buffer_storage_packet_num.push(packet.packet_num);
                else
                    buffer_storage_packet_num_a.push(packet.packet_num);
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
            if (buffer_storage_packet_num_a.size() >= QUEUE_BUFFER_SIZE) {
                while (buffer_storage_packet_num_a.size() > QUEUE_BUFFER_SIZE) {
                    buffer_storage_packet_num_a.pop();
                }

                lq = ((double)(buffer_storage_packet_num_a.size())) / ((double)(buffer_storage_packet_num_a.back() - buffer_storage_packet_num_a.front() + 1));
                printf("link quality: %f\n", lq);
                fprintf(outlinkquality, "%lu,%u,%u,%lu,%f\n", counter - 1, packet.senderID, packet.packet_num, current_time, lq);
            }


            fflush(NULL);
        }
    }
    else {
        for (int count_ = 0; count_ < numbers_of_packets; count_++) {
            if ((recvbytes = recvfrom(sockListen, &packet, sizeof(packet), 0, (struct sockaddr*)&recvAddr, &addrLen)) != -1) {
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
    }

    return 0;
}