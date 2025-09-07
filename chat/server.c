#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 10

int client_sockets[MAX_CLIENTS];
int client_count = 0;

void broadcast_message(int sender_fd, char *message) {
    for (int i = 0; i < client_count; i++) {
        if (client_sockets[i] != sender_fd) {
            send(client_sockets[i], message, strlen(message), 0);
        }
    }
}

void handle_client(int client_fd) {
    char buffer[BUFFER_SIZE];
    int n;

    while ((n = recv(client_fd, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[n] = '\0';
        printf("Message from client: %s\n", buffer);
        broadcast_message(client_fd, buffer);
    }

    // remove client from list
    for (int i = 0; i < client_count; i++) {
        if (client_sockets[i] == client_fd) {
            for (int j = i; j < client_count - 1; j++) {
                client_sockets[j] = client_sockets[j + 1];
            }
            client_count--;
            break;
        }
    }

    close(client_fd);
    exit(0);
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Socket creation failed");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(1);
    }

    if (listen(server_fd, 5) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(1);
    }

    printf("Chat server listening on port %d...\n", PORT);

    while (1) {
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
        if (client_fd < 0) {
            perror("Accept failed");
            continue;
        }

        if (client_count < MAX_CLIENTS) {
            client_sockets[client_count++] = client_fd;
        }

        if (fork() == 0) {  // child process
            close(server_fd); // child doesn’t need listening socket
            handle_client(client_fd);
        } else {
            close(client_fd); // parent doesn’t need connected socket
        }
    }

    close(server_fd);
    return 0;
}
