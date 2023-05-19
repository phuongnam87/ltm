#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <poll.h>

#define MAX_CLIENTS 10
#define MAX_MESSAGE_LEN 256

typedef struct {
    int fd;
    char name[MAX_MESSAGE_LEN];
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

    // Thiết lập poll
    struct pollfd fds[MAX_CLIENTS + 1];
    memset(fds, 0, sizeof(fds));
    fds[0].fd = server_fd;
    fds[0].events = POLLIN;
    int num_clients = 0;

    while (1) {
        // Sử dụng poll để xem các sự kiện trên các file descriptor
        int poll_count = poll(fds, num_clients + 1, -1);
        if (poll_count == -1) {
            perror("poll failed");
            exit(EXIT_FAILURE);
        }

        // Kiểm tra sự kiện trên server socket (kết nối mới)
        if (fds[0].revents & POLLIN) {
            if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
                perror("accept failed");
                exit(EXIT_FAILURE);
            }

            // Thêm client mới vào danh sách
            if (num_clients < MAX_CLIENTS) {
                fds[num_clients + 1].fd = new_socket;
                fds[num_clients + 1].events = POLLIN;
                clients[num_clients].fd = new_socket;
                num_clients++;
                printf("New client connected.\n");
            } else {
                printf("Maximum number of clients reached. Connection rejected.\n");
                close(new_socket);
            }
        }

        // Kiểm tra sự kiện trên các client socket (dữ liệu mới)
        for (i = 1; i <= num_clients; i++) {
            if (fds[i].revents & POLLIN) {
                char buffer[MAX_MESSAGE_LEN];

                // Nhận dữ liệu từ client
                int valread = recv(fds[i].fd, buffer, sizeof(buffer), 0);
                if (valread <= 0) {
                    // Kết nối đã đóng
                    close(fds[i].fd);

                    // Di chuyển client cuối cùng trong danh sách lên vị trí của client bị đóng
                    fds[i] = fds[num_clients];
                    clients[i - 1] = clients[num_clients - 1];
                    num_clients--;

                    printf("Client disconnected.\n");
                } else {
                    // Kiểm tra nếu chưa nhận tên của client
                    if (strlen(clients[i - 1].name) == 0) {
                        // Kiểm tra cú pháp tên client: "client_id: client_name"
                        char *token = strtok(buffer, ":");
                        if (token != NULL) {
                            strcpy(clients[i - 1].name, token + 1);  // Bỏ qua khoảng trắng đầu tiên
                            printf("Client name set: %s\n", clients[i - 1].name);
                        }
                    } else {
                        // Gửi dữ liệu nhận được đến các client khác
                        for (j = 1; j <= num_clients; j++) {
                            if (j != i) {
                                char message[MAX_MESSAGE_LEN + sizeof(clients[i - 1].name) + 20];  // +20 cho thời gian
                                snprintf(message, sizeof(message), "%s %s", clients[i - 1].name, buffer);

                                // Gửi dữ liệu
                                send(fds[j].fd, message, strlen(message), 0);
                            }
                        }
                    }
                }
            }
        }
    }

    return 0;
}
