[24bcs156@mepcolinux ex6]$cat chatserver1.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int createSocket();
void bindSocket(int server_fd, struct sockaddr_in *server_addr);
void listenForClient(int server_fd);
void chatWithClient(int client_fd);
void startServer();

int main()
{
    startServer();
    return 0;
}

// Create TCP socket
int createSocket()
{
    int server_fd;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0)
    {
        perror("Socket creation failed");
        exit(1);
    }

    return server_fd;
}

// Bind server socket
void bindSocket(int server_fd, struct sockaddr_in *server_addr)
{
    server_addr->sin_family = AF_INET;
    server_addr->sin_addr.s_addr = INADDR_ANY;
    server_addr->sin_port = htons(PORT);

    // Allow quick port reuse to prevent "Address already in use" errors
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (bind(server_fd,
             (struct sockaddr *)server_addr,
             sizeof(*server_addr)) < 0)
    {
        perror("Bind failed");
        close(server_fd);
        exit(1);
    }
}

// Listen for clients
void listenForClient(int server_fd)
{
    if (listen(server_fd, 5) < 0)
    {
        perror("Listen failed");
        close(server_fd);
        exit(1);
    }
}

// Chat with one client
void chatWithClient(int client_fd)
{
    char buffer[BUFFER_SIZE];

    printf("\n----------------------------------------\n");
    printf("     CHAT SESSION INITIALIZED           \n");
    printf("     Type 'exit' to end the session     \n");
    printf("----------------------------------------\n");

    while (1)
    {
        memset(buffer, 0, BUFFER_SIZE);

        int bytes = recv(client_fd,
                         buffer,
                         BUFFER_SIZE - 1,
                         0);

        if (bytes <= 0)
        {
            printf("\n----------------------------------------\n");
            printf(" Status: Client disconnected abruptly.\n");
            printf("----------------------------------------\n");
            break;
        }

        buffer[bytes] = '\0';

        // Check if incoming content has trailing newlines from raw clients
        buffer[strcspn(buffer, "\n\r")] = '\0';

        printf(" Client : %s\n", buffer);

        if (strcmp(buffer, "exit") == 0)
        {
            printf("\n----------------------------------------\n");
            printf(" Status: Session closed by client.\n");
            printf("----------------------------------------\n");
            break;
        }

        printf(" Server : ");
        fgets(buffer, BUFFER_SIZE, stdin);

        buffer[strcspn(buffer, "\n\r")] = '\0';

        send(client_fd,
             buffer,
             strlen(buffer),
             0);

        if (strcmp(buffer, "exit") == 0)
        {
            printf("\n----------------------------------------\n");
            printf(" Status: Session closed by server.\n");
            printf("----------------------------------------\n");
            break;
        }
        printf("----------------------------------------\n");
    }
}

// Start iterative server
void startServer()
{
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size;

    server_fd = createSocket();

    bindSocket(server_fd, &server_addr);

    listenForClient(server_fd);

    printf("========================================\n");
    printf("       TCP CHAT SERVER ONLINE           \n");
    printf("       Listening on Port: %d            \n", PORT);
    printf("========================================\n");
    printf("Waiting for an incoming connection...\n");

    // Iterative server
    while (1)
    {
        addr_size = sizeof(client_addr);

        client_fd = accept(server_fd,
                           (struct sockaddr *)&client_addr,
                           &addr_size);

        if (client_fd < 0)
        {
            perror("Accept failed");
            continue;
        }

        printf("\n========================================\n");
        printf(" SUCCESS: Client successfully connected!\n");
        printf("========================================\n");

        // Handle one client
        chatWithClient(client_fd);

        close(client_fd);

        printf("Waiting for next client...\n");
    }

    close(server_fd);
}