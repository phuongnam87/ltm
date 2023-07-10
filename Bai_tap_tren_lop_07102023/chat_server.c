
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

typedef struct {
    int socket_fd;
    struct sockaddr_in address;
    char username[20];
} Client;

Client clients[MAX_CLIENTS];
int num_clients = 0;

void broadcast_message(char *message, int sender_socket_fd) {
    for (int i = 0; i < num_clients; i++) {
        if (clients[i].socket_fd != sender_socket_fd) {
            write(clients[i].socket_fd, message, strlen(message));
        }
    }
}

void handle_client_message(int client_index, char *message) {
    char response[BUFFER_SIZE];

    // Handle different commands or messages here
    if (strcmp(message, "quit") == 0) {
        sprintf(response, "User '%s' has left the chat.\n", clients[client_index].username);
        broadcast_message(response, clients[client_index].socket_fd);
        close(clients[client_index].socket_fd);

        // Remove the client from the array
        for (int i = client_index; i < num_clients - 1; i++) {
            clients[i] = clients[i + 1];
        }
        num_clients--;
    } else {
        sprintf(response, "[%s]: %s", clients[client_index].username, message);
        broadcast_message(response, clients[client_index].socket_fd);
    }
}

void handle_client(int client_socket_fd) {
    char buffer[BUFFER_SIZE];
    int read_size;
    int client_index = -1;

    // Receive the client's username
    if ((read_size = recv(client_socket_fd, buffer, BUFFER_SIZE, 0)) > 0) {
        buffer[read_size] = '\0';
        printf("Received username: %s\n", buffer);

        // Add the client to the array
        Client new_client;
        new_client.socket_fd = client_socket_fd;
        new_client.address = clients[num_clients].address;
        strncpy(new_client.username, buffer, sizeof(new_client.username));
        clients[num_clients] = new_client;
        client_index = num_clients;
        num_clients++;

        // Send welcome message to the client
        char welcome_message[BUFFER_SIZE];
        sprintf(welcome_message, "Welcome, %s!\n", new_client.username);
        write(client_socket_fd, welcome_message, strlen(welcome_message));

        // Notify other clients about the new user
        char notification[BUFFER_SIZE];
        sprintf(notification, "User '%s' has joined the chat.\n", new_client.username);
        broadcast_message(notification, client_socket_fd);
    }

    // Handle messages from the client
    while ((read_size = recv(client_socket_fd, buffer, BUFFER_SIZE, 0)) > 0) {
        buffer[read_size] = '\0';
        handle_client_message(client_index, buffer);
    }
}

int main(int argc, char *argv[]) {
    int server_socket_fd, client_socket_fd;
    struct sockaddr_in server_address, client_address;
    unsigned short server_port;
    unsigned int client_address_length;

    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        exit(1);
    }

    server_port = atoi(argv[1]);

    // Create socket
    server_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket_fd < 0) {
        perror("Failed to create socket");
        exit(1);
    }

    // Prepare the server address structure
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(server_port);

    // Bind the socket to the server address
    if (bind(server_socket_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Failed to bind socket");
        exit(1);
    }

    // Start listening for incoming connections
    if (listen(server_socket_fd, MAX_CLIENTS) < 0) {
        perror("Error while listening");
        exit(1);
    }

    printf("Server started. Listening on port %d.\n", server_port);

    while (1) {
        // Accept a new connection
        client_address_length = sizeof(client_address);
        client_socket_fd = accept(server_socket_fd, (struct sockaddr *)&client_address, &client_address_length);
        if (client_socket_fd < 0) {
            perror("Error while accepting connection");
            exit(1);
        }

        printf("New client connected: %s:%d\n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));

        // Handle the client in a separate function
        handle_client(client_socket_fd);
    }

    return 0;
}
