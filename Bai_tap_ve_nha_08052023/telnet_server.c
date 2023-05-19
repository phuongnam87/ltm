#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

#define MAX_CLIENTS 10
#define MAX_USERNAME_LEN 50
#define MAX_PASSWORD_LEN 50
#define MAX_COMMAND_LEN 256

typedef struct {
    int fd;
    char username[MAX_USERNAME_LEN];
    char password[MAX_PASSWORD_LEN];
} Client;

int main() {
    int server_fd, new_socket, i, j;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    Client clients[MAX_CLIENTS];

    // Tạo socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Cấu hình địa chỉ và cổng của server
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    // Gắn địa chỉ và cổng vào socket
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Lắng nghe kết nối từ client
    if (listen(server_fd, 3) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    // Thiết lập tập file descriptor
    fd_set read_fds;
    int max_fd;

    // Thiết lập file descriptor của server
    int server_sock = server_fd;
    max_fd = server_sock;

    // Thiết lập tập file descriptor của các client
    int client_sockets[MAX_CLIENTS];
    for (i = 0; i < MAX_CLIENTS; i++) {
        client_sockets[i] = -1;
    }

    while (1) {
        // Thiết lập tập file descriptor cho select
        FD_ZERO(&read_fds);
        FD_SET(server_sock, &read_fds);

        for (i = 0; i < MAX_CLIENTS; i++) {
            if (client_sockets[i] != -1) {
                FD_SET(client_sockets[i], &read_fds);
            }

            if (client_sockets[i] > max_fd) {
                max_fd = client_sockets[i];
            }
        }

        // Sử dụng select để xem các sự kiện trên file descriptor
        int activity = select(max_fd + 1, &read_fds, NULL, NULL, NULL);
        if (activity < 0) {
            perror("select failed");
            exit(EXIT_FAILURE);
        }

        // Kiểm tra sự kiện trên server socket (kết nối mới)
        if (FD_ISSET(server_sock, &read_fds)) {
            if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
                perror("accept failed");
                exit(EXIT_FAILURE);
            }

            // Thêm client mới vào danh sách
            for (i = 0; i < MAX_CLIENTS; i++) {
                if (client_sockets[i] == -1) {
                    client_sockets[i] = new_socket;
                    printf("New client connected, socket fd is %d\n", new_socket);
                    break;
                }
            }
        }

        // Kiểm tra sự kiện trên các client socket (dữ liệu mới)
        for (i = 0; i < MAX_CLIENTS; i++) {
            int client_sock = client_sockets[i];
            if (client_sock != -1 && FD_ISSET(client_sock, &read_fds)) {
                char buffer[MAX_COMMAND_LEN];

                // Nhận dữ liệu từ client
                int valread = recv(client_sock, buffer, sizeof(buffer), 0);
                if (valread <= 0) {
                    // Đóng kết nối nếu gặp lỗi hoặc client đóng kết nối
                    close(client_sock);
                    client_sockets[i] = -1;
                    printf("Client disconnected, socket fd is %d\n", client_sock);
                    continue;
                }

                buffer[valread] = '\0';

                // Kiểm tra nếu chưa xác thực tài khoản
                if (clients[i].username[0] == '\0') {
                    char *token = strtok(buffer, " ");
                    if (token != NULL) {
                        strcpy(clients[i].username, token);

                        // Kiểm tra tài khoản
                        FILE *file = fopen("database.txt", "r");
                        if (file == NULL) {
                            perror("Failed to open database file");
                            exit(EXIT_FAILURE);
                        }

                        char line[MAX_USERNAME_LEN + MAX_PASSWORD_LEN];
                        int found = 0;
                        while (fgets(line, sizeof(line), file) != NULL) {
                            line[strcspn(line, "\n")] = '\0';
                            char *username = strtok(line, " ");
                            char *password = strtok(NULL, " ");
                            if (username != NULL && password != NULL && strcmp(username, clients[i].username) == 0 &&
                                strcmp(password, clients[i].password) == 0) {
                                found = 1;
                                break;
                            }
                        }

                        fclose(file);

                        if (found) {
                            send(client_sock, "Login successful\n", strlen("Login successful\n"), 0);
                        } else {
                            send(client_sock, "Invalid username or password\n", strlen("Invalid username or password\n"), 0);
                            close(client_sock);
                            client_sockets[i] = -1;
                            printf("Client disconnected, socket fd is %d\n", client_sock);
                            continue;
                        }
                    }
                } else {
                    // Thực hiện lệnh từ client và trả kết quả
                    char command[MAX_COMMAND_LEN + 10];  // +10 để thêm " > out.txt"
                    snprintf(command, sizeof(command), "%s > out.txt", buffer);
                    system(command);

                    // Đọc kết quả từ file
                    FILE *file = fopen("out.txt", "r");
                    if (file == NULL) {
                        perror("Failed to open output file");
                        exit(EXIT_FAILURE);
                    }

                    char output[MAX_COMMAND_LEN];
                    memset(output, 0, sizeof(output));
                    size_t len = 0;
                    ssize_t read;
                    while ((read = getline(&output, &len, file)) != -1) {
                        send(client_sock, output, strlen(output), 0);
                        memset(output, 0, sizeof(output));
                    }

                    fclose(file);
                }
            }
        }
    }

    return 0;
}
