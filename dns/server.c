#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main()
{
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    char buffer[BUFFER_SIZE];
    socklen_t addr_len = sizeof(client_addr);

    // Create UDP socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("Socket creation failed");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    memset(&client_addr, 0, sizeof(client_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind the socket
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Bind failed");
        close(sockfd);
        exit(1);
    }

    printf("DNS server listening on port %d...\n", PORT);

    while (1)
    {
        int n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0,
                         (struct sockaddr *)&client_addr, &addr_len);
        buffer[n] = '\0';

        printf("Query received: %s\n", buffer);

        // Resolve hostname
        struct hostent *he = gethostbyname(buffer);
        char reply[BUFFER_SIZE];

        if (he == NULL)
        {
            snprintf(reply, sizeof(reply), "Host not found");
        }
        else
        {
            struct in_addr **addr_list = (struct in_addr **)he->h_addr_list;
            if (addr_list[0] != NULL)
            {
                snprintf(reply, sizeof(reply), "%s", inet_ntoa(*addr_list[0]));
            }
            else
            {
                snprintf(reply, sizeof(reply), "No IP found");
            }
        }

        // Send response to client
        sendto(sockfd, reply, strlen(reply), 0,
               (struct sockaddr *)&client_addr, addr_len);
        printf("Replied with: %s\n", reply);
    }

    close(sockfd);
    return 0;
}
