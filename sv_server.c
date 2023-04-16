#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>

#define MAX_MSG_LENGTH 1024

typedef struct {
    char mssv[10];
    char name[50];
    char dob[20];
    float gpa;
} Student;

void error(char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <port> <log_file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        error("ERROR opening socket");
    }

    struct sockaddr_in server_addr, client_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(atoi(argv[1]));

    if (bind(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        error("ERROR on binding");
    }

    listen(sockfd, 5);

    socklen_t client_len = sizeof(client_addr);
    int newsockfd = accept(sockfd, (struct sockaddr *) &client_addr, &client_len);
    if (newsockfd < 0) {
        error("ERROR on accept");
    }

    char buffer[MAX_MSG_LENGTH];
    memset(buffer, 0, sizeof(buffer));

    ssize_t n = read(newsockfd, buffer, sizeof(buffer));
    if (n < 0) {
        error("ERROR reading from socket");
    }

    Student student;
    memcpy(&student, buffer, sizeof(student));

    FILE *log_file = fopen(argv[2], "a");
    if (log_file == NULL) {
        error("ERROR opening log file");
    }

    time_t now = time(NULL);
    struct tm *tm_now = localtime(&now);
    char time_str[20];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_now);

    printf("%s %s %s %.2f\n", student.mssv, student.name, student.dob, student.gpa);
    fprintf(log_file, "%s %s %s %s %s %.2f\n", inet_ntoa(client_addr.sin_addr), time_str, student.mssv, student.name, student.dob, student.gpa);

    fclose(log_file);
    close(newsockfd);
    close(sockfd);

    return EXIT_SUCCESS;
}
