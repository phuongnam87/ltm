#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/select.h>

#define SERVER_ADDR "127.0.0.1"
#define SERVER_PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(SERVER_ADDR);
    server_address.sin_port = htons(SERVER_PORT);

    if (connect(sock, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        perror("connection failed");
        exit(EXIT_FAILURE);
    }

    // Nhập tên của client và gửi tên đến server
    char client_id[256];
    char client_name[256];
    printf("Enter your client: ");
    fgets(client_name, sizeof(client_name), stdin);
    // sprintf(client_id, "client%d", sock);

    char message[BUFFER_SIZE];
    sprintf(message, "%s", client_name);
    write(sock, message, strlen(message));

    fd_set readfds;

    char *token = strtok(client_name, ":");
    memset(client_id, 0, sizeof(client_id));

    // Lấy client_id và client_name từ buffer
    sscanf(token, "%s", client_id);

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(sock, &readfds);
        FD_SET(STDIN_FILENO, &readfds);

        int max_fd = sock > STDIN_FILENO ? sock : STDIN_FILENO;
        select(max_fd + 1, &readfds, NULL, NULL, NULL);

        if (FD_ISSET(sock, &readfds)) {
            char buffer[BUFFER_SIZE];
            int valread = read(sock, buffer, BUFFER_SIZE);
            printf("%s", buffer);
            memset(buffer, 0 ,sizeof(message));
        }

        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            char buffer[BUFFER_SIZE];
            fgets(buffer, sizeof(buffer), stdin);

            if (strcmp(buffer, "exit\n") == 0) {
                break;
            }

            sprintf(message, "%s:%s", client_id , buffer);
            write(sock, message, strlen(message));
            memset(message, 0 ,sizeof(message));
            memset(buffer, 0 ,sizeof(message));
        }
    }

    close(sock);

    return 0;
}
