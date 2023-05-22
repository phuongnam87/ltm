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
    if (argc < 4) {
        printf("Usage: %s <receiver_ip> <receiver_port> <sender_port>\n", argv[0]);
        exit(1);
    }

    char *receiver_ip = argv[1];
    int receiver_port = atoi(argv[2]);
    int sender_port = atoi(argv[3]);

    int sender_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (sender_socket < 0) {
        perror("Error creating sender socket");
        exit(1);
    }

    struct sockaddr_in receiver_addr;
    memset(&receiver_addr, 0, sizeof(receiver_addr));

    receiver_addr.sin_family = AF_INET;
    receiver_addr.sin_port = htons(receiver_port);
    receiver_addr.sin_addr.s_addr = inet_addr(receiver_ip);

    char buffer[MAX_BUFFER_SIZE];

    while (1) {
        printf("Enter message: ");
        fgets(buffer, sizeof(buffer), stdin);

        ssize_t send_len = sendto(sender_socket, buffer, strlen(buffer), 0, (struct sockaddr *)&receiver_addr, sizeof(receiver_addr));
        if (send_len < 0) {
            perror("Error sending message");
            exit(1);
        }
    }

    close(sender_socket);
    return 0;
}
