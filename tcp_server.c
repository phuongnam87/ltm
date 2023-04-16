#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    int server_socket, client_socket, file_fd, len;
    struct sockaddr_in server_addr, client_addr;
    char buffer[BUFFER_SIZE];

    // Kiểm tra số lượng tham số dòng lệnh
    if (argc != 4) {
        printf("Sử dụng: %s <cổng> <tệp tin chứa câu chào> <tệp tin lưu nội dung client gửi đến>\n", argv[0]);
        return 1;
    }

    // Mở tệp tin chứa câu chào và đọc nội dung vào buffer
    file_fd = open(argv[2], O_RDONLY);
    if (file_fd < 0) {
        perror("Lỗi khi đọc tệp tin chứa câu chào");
        return 1;
    }
    len = read(file_fd, buffer, BUFFER_SIZE);
    if (len < 0) {
        perror("Lỗi khi đọc nội dung tệp tin chứa câu chào");
        return 1;
    }
    close(file_fd);

    // Tạo socket cho server
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Lỗi khi tạo socket cho server");
        return 1;
    }

    // Cấu hình địa chỉ cho server
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(atoi(argv[1]));

    // Bind socket với địa chỉ server
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Lỗi khi bind socket");
        return 1;
    }

    // Đợi kết nối từ client
    listen(server_socket, MAX_CLIENTS);
    printf("Đang đợi kết nối từ client...\n");

    // Chấp nhận kết nối từ client và gửi câu chào
    socklen_t client_len = sizeof(client_addr);
    client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
    if (client_socket < 0) {
        perror("Lỗi khi chấp nhận kết nối từ client");
        return 1;
    }
    printf("Đã kết nối với client %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    send(client_socket, buffer, len, 0);
    // Mở tệp tin để ghi nội dung client gửi đến
    file_fd = open(argv[3], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (file_fd < 0) {
        perror("Lỗi khi mở tệp tin để ghi nội dung client gửi đến");
        return 1;
    }

    // Đọc nội dung client gửi đến và ghi vào tệp tin
    while ((len = recv(client_socket, buffer, BUFFER_SIZE, 0)) > 0) {
        write(file_fd, buffer, len);
    }

    if (len < 0) {
        perror("Lỗi khi đọc nội dung client gửi đến");
        return 1;
    }

    printf("Đã nhận được nội dung từ client %s:%d và lưu vào tệp tin %s\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), argv[3]);

    // Đóng socket và tệp tin
    close(client_socket);
    close(server_socket);
    close(file_fd);

    return 0;
}

