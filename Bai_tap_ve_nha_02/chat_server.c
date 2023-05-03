#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

int c = 0;

int main(int argc, char *argv[]) {
    // Khởi tạo server socket
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Không thể khởi tạo socket");
        exit(EXIT_FAILURE);
    }

    // Đặt cấu hình địa chỉ
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(8080);

    // Gắn địa chỉ vào socket
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        perror("Không thể bind địa chỉ");
        exit(EXIT_FAILURE);
    }

    // Listen cho các kết nối đến
    if (listen(server_socket, MAX_CLIENTS) == -1) {
        perror("Lỗi khi lắng nghe kết nối");
        exit(EXIT_FAILURE);
    }

    // Đặt các client sockets vào một mảng và đặt set cho hàm select
    int client_sockets[MAX_CLIENTS];
    fd_set readfds;

    for (int i = 0; i < MAX_CLIENTS; i++) {
        client_sockets[i] = 0;
    }

    // Chờ các kết nối đến và xử lý chúng
    printf("Server đang chạy...\n");

    while (1) {
        // Đặt set cho hàm select
        FD_ZERO(&readfds);
        FD_SET(server_socket, &readfds);

        int max_sd = server_socket;

        for (int i = 0; i < MAX_CLIENTS; i++) {
            int sd = client_sockets[i];

            if (sd > 0) {
                FD_SET(sd, &readfds);
            }

            if (sd > max_sd) {
                max_sd = sd;
            }
        }

        // Sử dụng hàm select để theo dõi các kết nối đến
        int activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        if (activity == -1) {
            perror("Lỗi khi sử dụng hàm select");
            exit(EXIT_FAILURE);
        }

        // Nếu có kết nối mới, chấp nhận kết nối đó và thêm vào set
        if (FD_ISSET(server_socket, &readfds)) {
            int client_socket = accept(server_socket, NULL, NULL);

            if (client_socket == -1) {
                perror("Lỗi khi chấp nhận kết nối");
                exit(EXIT_FAILURE);
            }

            printf("New client connected\n");

            // Thêm client vào mảng
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (client_sockets[i] == 0) {
                    client_sockets[i] = client_socket;
                    break;
                }
            }
        }

        // Xử lý các kết nối hiện tại
        for (int i = 0; i < MAX_CLIENTS; i++) {
            int sd = client_sockets[i];

            // Kiểm tra xem socket có sẵn sàng để đọc không
            if (FD_ISSET(sd, &readfds)) {
                char buffer[BUFFER_SIZE];
                int valread = read(sd, buffer, BUFFER_SIZE);

                if (valread == 0) {
                    // Ngắt kết nối nếu client đã đóng kết nối
                    printf("Client disconnected\n");
                    close(sd);
                    client_sockets[i] = 0;
                } else {
                    // Nhận tên của client
                    // if (strlen(buffer) > 0 && strchr(buffer, ':') != NULL) {
                    if (c == 0) {
                        char *token = strtok(buffer, ":");
                        char client_id[256];
                        char client_name[256];
                        memset(client_id, 0, sizeof(client_id));
                        memset(client_name, 0, sizeof(client_name));
                    
                        // Lấy client_id và client_name từ buffer
                        sscanf(token, "%s", client_id);
                        token = strtok(NULL, ":");
                        sscanf(token, "%s", client_name);

                        // In ra thông tin của client mới kết nối
                        printf("Client %s connected with name %s\n", client_id, client_name);

                        // Gửi thông báo cho các client khác
                        char message[BUFFER_SIZE];
                        sprintf(message, "%s: connected\n", client_name);

                        for (int j = 0; j < MAX_CLIENTS; j++) {
                            int other_sd = client_sockets[j];

                            if (other_sd != 0 && other_sd != sd) {
                                write(other_sd, message, strlen(message));
                            }
                        }
                        memset(message, 0 ,sizeof(message));
                        c = 1;

                    } else {

                        // Gửi tin nhắn cho các client khác
                        char message[BUFFER_SIZE];
                        sprintf(message, "%s\n", buffer);

                        for (int j = 0; j < MAX_CLIENTS; j++) {
                            int other_sd = client_sockets[j];

                            if (other_sd != 0 && other_sd != sd) {
                                write(other_sd, message, strlen(message));
                            }
                        }
                        memset(message, 0 ,sizeof(message));
                    }

                    
                }

                memset(buffer, 0 ,sizeof(buffer));
            }
        }
    }

    return 0;
}