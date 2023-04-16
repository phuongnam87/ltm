#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUF_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <IP address> <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // tạo socket
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0) {
        perror("socket() failed");
        exit(EXIT_FAILURE);
    }

    // tạo địa chỉ máy chủ
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);
    server_addr.sin_port = htons(atoi(argv[2]));

    // kết nối đến máy chủ
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect() failed");
        exit(EXIT_FAILURE);
    }

    // nhận dữ liệu từ bàn phím và gửi đến server
    char buf[BUF_SIZE];

    int recv_len = recv(sock, buf, BUF_SIZE - 1, 0);
    if (recv_len < 0) {
        perror("recv() failed");
        exit(EXIT_FAILURE);
    } else if (recv_len == 0) {
        printf("Connection closed by server\n");
        return 0;
    } else {
        buf[recv_len] = '\0';
        printf("Received: %s\n", buf);
    }

    while (1) {
        printf("Enter message: ");
        fgets(buf, BUF_SIZE, stdin);
        buf[strcspn(buf, "\n")] = '\0'; // xóa ký tự xuống dòng
        if (strlen(buf) == 0) continue;

        if (send(sock, buf, strlen(buf), 0) < 0) {
            perror("send() failed");
            exit(EXIT_FAILURE);
        }
    }

    close(sock);
    exit(EXIT_SUCCESS);
}
