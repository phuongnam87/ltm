#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 1024

void error_handling(char *message);

int main(int argc, char *argv[]) {
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_addr, clnt_addr;
    socklen_t clnt_addr_size;
    char buf[BUF_SIZE];
    int str_len, recv_len, recv_cnt;
    int cnt_0123456789 = 0;
    char prev_buf[BUF_SIZE] = "";
    char *pos, *start;

    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (serv_sock == -1) error_handling("socket() error");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));

    if (bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) error_handling("bind() error");
    if (listen(serv_sock, 5) == -1) error_handling("listen() error");

    clnt_addr_size = sizeof(clnt_addr);
    clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
    if (clnt_sock == -1) error_handling("accept() error");

    while ((recv_len = recv(clnt_sock, buf, BUF_SIZE - 1, 0)) != 0) {
        if (recv_len == -1) error_handling("recv() error");

        buf[recv_len] = '\0';
        if (strlen(prev_buf) > 0) {
            strncat(prev_buf, buf, BUF_SIZE - strlen(prev_buf) - 1);
            start = prev_buf;
        } else {
            start = buf;
        }

        while ((pos = strstr(start, "0123456789")) != NULL) {
            cnt_0123456789++;
            start = pos + strlen("0123456789");
        }

        strcpy(prev_buf, start);
    }

    printf("The number of occurrences of \"0123456789\" is: %d\n", cnt_0123456789);

    close(clnt_sock);
    close(serv_sock);

    return 0;
}

void error_handling(char *message) {
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
