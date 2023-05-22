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
        printf("Usage: %s <server_ip> <server_port>\n", argv[0]);
        exit(1);
    }

    char *server_ip = argv[1];
    int server_port = atoi(argv[2]);

    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        perror("Error creating client socket");
        exit(1);
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    server_addr.sin_addr.s_addr = inet_addr(server_ip);

    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error connecting to server");
        exit(1);
    }

    char server_msg[MAX_BUFFER_SIZE];
    ssize_t recv_len = recv(client_socket, server_msg, sizeof(server_msg), 0);
    if (recv_len < 0) {
        perror("Error receiving message from server");
        exit(1);
    }
    server_msg[recv_len] = '\0';
    printf("%s", server_msg);

    char client_msg[MAX_BUFFER_SIZE];

    while (1) {
        printf("Enter message (or 'exit' to quit): ");
        fgets(client_msg, sizeof(client_msg), stdin);

        if (send(client_socket, client_msg, strlen(client_msg), 0) < 0) {
            perror("Error sending message to server");
            exit(1);
        }

        if (strcmp(client_msg, "exit\n") == 0) {
            break;
        }

        ssize_t recv_len = recv(client_socket, server_msg, sizeof(server_msg), 0);
        if (recv_len < 0) {
            perror("Error receiving message from server");
            exit(1);
        }
        server_msg[recv_len] = '\0';
        printf("Server response: %s", server_msg);
    }

    close(client_socket);
    return 0;
}
