#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <time.h>
#include <stdint.h>
#include <inttypes.h>

#include "err.h"

#define BUFFER_SIZE 100

int main(int argc, char *argv[]) {
    int tcp;

    if (argc < 4 || (strcmp(argv[1], "-t") && strcmp(argv[1], "-u"))) {
        fprintf(stderr, "Usage: %s [-t|-u] host port\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (strcmp(argv[1], "-t") == 0)
        tcp = 1;
    else
        tcp = 0;

    int sock, err;
    struct addrinfo addr_hints;
    struct addrinfo *addr_result;

    memset(&addr_hints, 0, sizeof(struct addrinfo));
    addr_hints.ai_family = AF_INET;

    if (tcp) {
        addr_hints.ai_socktype = SOCK_STREAM;
        addr_hints.ai_protocol = IPPROTO_TCP;
        err = getaddrinfo(argv[2], argv[3], &addr_hints, &addr_result);
        if (err != 0)
            syserr("getaddrinfo: %s\n", gai_strerror(err));

        sock = socket(addr_result->ai_family, addr_result->ai_socktype, addr_result->ai_protocol);
        if (sock < 0)
            syserr("socket tcp");
        
        clock_t start = clock();

        if (connect(sock, addr_result->ai_addr, addr_result->ai_addrlen) < 0) {
            syserr("connect");
        }

        clock_t end = clock();

        float seconds = (float)(end - start) / CLOCKS_PER_SEC;
        printf("Connection took %f seconds.\n", seconds);

        freeaddrinfo(addr_result);

       if (close(sock) == -1) {
           syserr("close tcp");
       }
    } else {
        int flags, sflags;
        size_t len;
        ssize_t rcv_len, snd_len;
        socklen_t rcva_len;
        struct sockaddr_in my_address;
        struct sockaddr_in srvr_address;
        char buffer[BUFFER_SIZE];
    
        addr_hints.ai_socktype = SOCK_DGRAM;
        addr_hints.ai_protocol = IPPROTO_UDP;
        addr_hints.ai_flags = 0;
        addr_hints.ai_addr = NULL;
        addr_hints.ai_canonname = NULL;
        addr_hints.ai_next = NULL;
        err = getaddrinfo(argv[2], NULL, &addr_hints, &addr_result);
        if (err != 0)
            syserr("getaddrinfo: %s\n", gai_strerror(err));

        my_address.sin_family = AF_INET;
        my_address.sin_addr.s_addr = ((struct sockaddr_in*) (addr_result->ai_addr))->sin_addr.s_addr; // address IP
        my_address.sin_port = htons((uint16_t) atoi(argv[3])); // port

        freeaddrinfo(addr_result);

        sock = socket(PF_INET, SOCK_DGRAM, 0);

        if (sock < 0)
            syserr("socket udp");

        sflags = 0;
        rcva_len = (socklen_t) sizeof(my_address);

        struct timespec tms;
        if (clock_gettime(CLOCK_REALTIME, &tms))
            syserr("clock_gettime");

        uint64_t microseconds = tms.tv_sec * 1000000; // Seconds -> microseconds
        microseconds += tms.tv_nsec / 1000; // Adding microseconds (from nanoseconds)

        if (tms.tv_nsec % 1000 >= 500) // Rounding up if necessary
            ++microseconds;

        sprintf(buffer, "%"PRId64"", microseconds);
        len = strlen(buffer);

        fprintf(stderr, "Sending %zd bytes: '%s'\n", len, buffer);

        snd_len = sendto(sock, buffer, len, sflags,
                (struct sockaddr *) &my_address, rcva_len);

        clock_t start = clock();

        if (snd_len != (ssize_t) len) {
            syserr("partial / failed write");
        }

        memset(buffer, 0, sizeof(buffer));
        flags = 0;
        len = (size_t) sizeof(buffer) - 1;
        rcva_len = (socklen_t) sizeof(srvr_address);
        rcv_len = recvfrom(sock, buffer, len, flags,
                (struct sockaddr *) &srvr_address, &rcva_len);

        clock_t end = clock();

        if (rcv_len < 0) {
            syserr("read");
        }

        float seconds = (float)(end - start) / CLOCKS_PER_SEC;
        printf("Waited %f seconds for an answer.\n", seconds);

        fprintf(stderr, "Received %zd bytes: '%s'\n", rcv_len, buffer);

        if (close(sock) == -1) {
            syserr("close udp");
        }
    }
    return 0;
}
