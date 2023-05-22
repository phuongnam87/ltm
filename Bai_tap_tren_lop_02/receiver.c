#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAX_BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s <receiver_port> <sender_port>\n", argv[0]);
        exit(1);
    }

    int receiver_port = atoi(argv[1]);
    int sender_port = atoi(argv[2]);

    int receiver_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (receiver_socket < 0) {
        perror("Error creating receiver socket");
        exit(1);
    }

    struct sockaddr_in receiver_addr, sender_addr;
    memset(&receiver_addr, 0, sizeof(receiver_addr));
    memset(&sender_addr, 0, sizeof(sender_addr));

    receiver_addr.sin_family = AF_INET;
    receiver_addr.sin_port = htons(receiver_port);
    receiver_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    sender_addr.sin_family = AF_INET;
    sender_addr.sin_port = htons(sender_port);
    sender_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (bind(receiver_socket, (struct sockaddr *)&receiver_addr, sizeof(receiver_addr)) < 0) {
        perror("Error binding receiver socket");
        exit(1);
    }

    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(receiver_socket, &read_fds);

    while (1) {
        fd_set tmp_fds = read_fds;
        if (select(receiver_socket + 1, &tmp_fds, NULL, NULL, NULL) == -1) {
            perror("Error in select");
            exit(1);
        }

        if (FD_ISSET(receiver_socket, &tmp_fds)) {
            char buffer[MAX_BUFFER_SIZE];
            struct sockaddr_in sender_addr;
            socklen_t sender_addr_len = sizeof(sender_addr);
            ssize_t recv_len = recvfrom(receiver_socket, buffer, sizeof(buffer), 0, (struct sockaddr *)&sender_addr, &sender_addr_len);
            if (recv_len < 0) {
                perror("Error receiving message");
                exit(1);
            }
            buffer[recv_len] = '\0';
            printf("Received message from %s:%d: %s\n", inet_ntoa(sender_addr.sin_addr), ntohs(sender_addr.sin_port), buffer);
        }
    }

    close(receiver_socket);
    return 0;
}
