#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>

#define MAX_CLIENTS 10
#define MAX_BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    int server_port = atoi(argv[1]);

    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Error creating server socket");
        exit(1);
    }

    struct sockaddr_in server_addr, client_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    memset(&client_addr, 0, sizeof(client_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error binding server socket");
        exit(1);
    }

    if (listen(server_socket, MAX_CLIENTS) < 0) {
        perror("Error listening on server socket");
        exit(1);
    }

    fd_set master_fds, read_fds;
    int fd_max = server_socket;
    FD_ZERO(&master_fds);
    FD_ZERO(&read_fds);
    FD_SET(server_socket, &master_fds);

    int num_clients = 0;
    char client_msg[MAX_BUFFER_SIZE];
    char server_msg[MAX_BUFFER_SIZE];

    printf("Server is running on port %d\n", server_port);

    while (1) {
        read_fds = master_fds;
        if (select(fd_max + 1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("Error in select");
            exit(1);
        }

        for (int fd = 0; fd <= fd_max; ++fd) {
            if (FD_ISSET(fd, &read_fds)) {
                if (fd == server_socket) {
                    // New client connection
                    socklen_t client_addr_len = sizeof(client_addr);
                    int client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
                    if (client_socket < 0) {
                        perror("Error accepting client connection");
                        exit(1);
                    }

                    FD_SET(client_socket, &master_fds);
                    if (client_socket > fd_max)
                        fd_max = client_socket;

                    ++num_clients;
                    sprintf(server_msg, "Xin chào. Hiện có %d client đang kết nối.\n", num_clients);
                    if (send(client_socket, server_msg, strlen(server_msg), 0) < 0) {
                        perror("Error sending message to client");
                        exit(1);
                    }

                    printf("New client connected from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                } else {
                    // Client message
                    ssize_t recv_len = recv(fd, client_msg, sizeof(client_msg), 0);
                    if (recv_len <= 0) {
                        if (recv_len == 0) {
                            // Client closed connection
                            printf("Client on socket %d closed the connection\n", fd);
                        } else {
                            perror("Error receiving message from client");
                        }

                        close(fd);
                        FD_CLR(fd, &master_fds);
                        --num_clients;
                        continue;
                    }

                    client_msg[recv_len] = '\0';

                    if (strcmp(client_msg, "exit") == 0) {
                        sprintf(server_msg, "Tạm biệt. Đã đóng kết nối.\n");
                        if (send(fd, server_msg, strlen(server_msg), 0) < 0) {
                            perror("Error sending message to client");
                            exit(1);
                        }

                        printf("Client on socket %d requested to exit\n", fd);

                        close(fd);
                        FD_CLR(fd, &master_fds);
                        --num_clients;
                        continue;
                    }

                    // Normalize client message
                    int i, j;
                    for (i = 0, j = 0; i < recv_len; ++i) {
                        if (isspace(client_msg[i])) {
                            if (i > 0 && !isspace(client_msg[i - 1])) {
                                client_msg[j++] = ' ';
                            }
                        } else if (isalpha(client_msg[i])) {
                            if (i == 0 || (i > 0 && isspace(client_msg[i - 1]))) {
                                client_msg[j++] = toupper(client_msg[i]);
                            } else {
                                client_msg[j++] = tolower(client_msg[i]);
                            }
                        }
                    }
                    client_msg[j] = '\0';

                    if (send(fd, client_msg, strlen(client_msg), 0) < 0) {
                        perror("Error sending message to client");
                        exit(1);
                    }

                    printf("Client on socket %d sent message: %s\n", fd, client_msg);
                }
            }
        }
    }

    close(server_socket);
    return 0;
}
