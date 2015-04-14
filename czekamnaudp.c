#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <stdint.h>
#include <inttypes.h>

#include "err.h"

#define BUFFER_SIZE   1000

int main(int argc, char *argv[]) {

    if (argc < 2) {
        fprintf(stderr, "Usage: %s port\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int sock;
    int flags, sflags;
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;

    char buffer[BUFFER_SIZE];
    socklen_t snda_len, rcva_len;
    ssize_t len, snd_len;

    sock = socket(AF_INET, SOCK_DGRAM, 0); // creating IPv4 UDP socket
    if (sock < 0)
        syserr("socket");
    // after socket() call; we should close(sock) on any execution path;
    // since all execution paths exit immediately, sock would be closed when program terminates

    server_address.sin_family = AF_INET; // IPv4
    server_address.sin_addr.s_addr = htonl(INADDR_ANY); // listening on all interfaces
    server_address.sin_port = htons(atoi(argv[1])); // default port for receiving is PORT_NUM

    // bind the socket to a concrete address
    if (bind(sock, (struct sockaddr *) &server_address,
                (socklen_t) sizeof(server_address)) < 0)
        syserr("bind");

    snda_len = (socklen_t) sizeof(client_address);
    for(;;) {
        memset(buffer, 0, sizeof(buffer));
        rcva_len = (socklen_t) sizeof(client_address);
        flags = 0; // we do not request anything special
        len = recvfrom(sock, buffer, sizeof(buffer), flags,
                (struct sockaddr *) &client_address, &rcva_len);

        if (len < 0)
            syserr("error on datagram from client socket");
        else {
            (void) printf("read from socket: %zd bytes: %.*s\n", 
                    len, (int) len, buffer);

            struct timespec tms;
            if (clock_gettime(CLOCK_REALTIME, &tms))
                syserr("clock_gettime");

            uint64_t microseconds = tms.tv_sec * 1000000;
            microseconds += tms.tv_nsec / 1000;

            if (tms.tv_nsec % 1000 >= 500)
                ++microseconds;
            char buffer2[BUFFER_SIZE];

            sprintf(buffer2, "%s %"PRId64"", buffer, microseconds);
            len = strlen(buffer2);
            printf("%s\n", buffer2);

            sflags = 0;
            snd_len = sendto(sock, buffer2, (size_t) len, sflags,
                    (struct sockaddr *) &client_address, snda_len);
            if (snd_len != len)
                syserr("error on sending datagram to client socket");
        }
        (void) printf("finished exchange\n");
    }

    return 0;
}
