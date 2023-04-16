#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

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
        fprintf(stderr, "Usage: %s <server_ip> <server_port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        error("ERROR opening socket");
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);
    server_addr.sin_port = htons(atoi(argv[2]));

    if (connect(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        error("ERROR connecting");
    }

    Student student;
    printf("MSSV: ");
    fgets(student.mssv, sizeof(student.mssv), stdin);
    printf("Name: ");
    fgets(student.name, sizeof(student.name), stdin);
    printf("DOB: ");
    fgets(student.dob, sizeof(student.dob), stdin);
    printf("GPA: ");
    scanf("%f", &student.gpa);

    char buffer[MAX_MSG_LENGTH];
    memset(buffer, 0, sizeof(buffer));
    memcpy(buffer, &student, sizeof(student));

    ssize_t n = write(sockfd, buffer, sizeof(buffer));
    if (n < 0) {
        error("ERROR writing to socket");
    }

    close(sockfd);

    return EXIT_SUCCESS;
}
