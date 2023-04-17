#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 1024

void error_handling(char *message);

int main(int argc, char *argv[]) {
    int sock;
    struct sockaddr_in serv_addr;
    char buf[BUF_SIZE];
    FILE *fp;
    int read_cnt, send_cnt, send_len, total_send_len;

    if (argc != 3) {
        printf("Usage: %s <IP> <port>\n", argv[0]);
        exit(1);
    }

    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1) error_handling("socket() error");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) error_handling("connect() error");

    fp = fopen("test.txt", "rb");
    if (fp == NULL) error_handling("fopen() error");
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    rewind(fp);
    char *file_buffer = (char*)malloc(sizeof(char) * file_size);
    fread(file_buffer, sizeof(char), file_size, fp);

    total_send_len = 0;
    while (total_send_len < file_size) {
        send_len = BUF_SIZE < file_size - total_send_len ? BUF_SIZE : file_size - total_send_len;
        send_cnt = send(sock, file_buffer + total_send_len, send_len, 0);
        if (send_cnt == -1) error_handling("send() error");
        total_send_len += send_cnt;
    }

    read_cnt = recv(sock, buf, BUF_SIZE - 1, 0);
    if (read_cnt == -1) error_handling("recv() error");
    buf[read_cnt] = '\0';
    printf("Received message from server: %s\n", buf);

    fclose(fp);
    free(file_buffer);
    close(sock);

    return 0;
}

void error_handling(char *message) {
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
