#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8888

int main(int argc, char *argv[]) {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char hostname[50], drives[100], buffer[1024];
    
    // Nhập tên máy tính và danh sách các ổ đĩa từ bàn phím
    printf("Nhập tên máy tính: ");
    scanf("%s", hostname);
    printf("Số ổ đĩa: ");
    int n;
    scanf("%d", &n);
    printf("Nhập danh sách các ổ đĩa (mỗi ổ đĩa gồm ký tự và kích thước, phân tách bởi dấu cách):\n");
    scanf(" %[^\n]s", drives);
    
    // Kết nối đến info_server
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }
    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }
    
    // Gửi dữ liệu tới info_server
    snprintf(buffer, sizeof(buffer), "%s;%d;%s", hostname, n, drives);
    send(sock, buffer, strlen(buffer), 0);
    printf("Đã gửi thông tin đến info_server.\n");
    
    close(sock);
    return 0;
}
