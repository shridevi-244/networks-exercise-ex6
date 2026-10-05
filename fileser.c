#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8081
#define BUFFER_SIZE 1024

int createSocket();
void bindServer(int server_fd, struct sockaddr_in *server_addr);
void startListening(int server_fd);
void handleClient(int client_fd);
void startServer();

int main()
{
    startServer();
    return 0;
}

/* Create TCP socket */
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

/* Bind server */
void bindServer(int server_fd,
                struct sockaddr_in *server_addr)
{
    server_addr->sin_family = AF_INET;
    server_addr->sin_addr.s_addr = INADDR_ANY;
    server_addr->sin_port = htons(PORT);

    /* SOLVES BIND FAILED ERROR: Allows immediate port reuse */
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

/* Listen for clients */
void startListening(int server_fd)
{
    if (listen(server_fd, 5) < 0)
    {
        perror("Listen failed");
        close(server_fd);
        exit(1);
    }
}

/* Handle one client */
void handleClient(int client_fd)
{
    char filename[BUFFER_SIZE];
    char buffer[BUFFER_SIZE];
    FILE *fp;
    int bytes;

    /* Receive filename */
    memset(filename, 0, BUFFER_SIZE);

    bytes = recv(client_fd,
                 filename,
                 BUFFER_SIZE - 1,
                 0);

    if (bytes <= 0)
    {
        printf("\n----------------------------------------\n");
        printf(" Status: Client disconnected abruptly.\n");
        printf("----------------------------------------\n");
        return;
    }

    filename[bytes] = '\0';

    printf("\n----------------------------------------\n");
    printf(" Requested File : %s\n", filename);
    printf("----------------------------------------\n");

    /* Open file */
    fp = fopen(filename, "r");

    if (fp == NULL)
    {
        strcpy(buffer, "File not present");

        send(client_fd,
             buffer,
             strlen(buffer),
             0);

        printf(" Status         : FILE NOT PRESENT\n");
        printf("----------------------------------------\n");
    }
    else
    {
        strcpy(buffer, "File present");

        send(client_fd,
             buffer,
             strlen(buffer),
             0);

        /* Send file contents */
        while (1)
        {
            bytes = fread(buffer,
                          1,
                          BUFFER_SIZE,
                          fp);

            if (bytes <= 0)
                break;

            send(client_fd,
                 buffer,
                 bytes,
                 0);
        }

        printf(" Status         : TRANSFERRED SUCCESSFULLY\n");
        printf("----------------------------------------\n");

        fclose(fp);
    }
}

/* Start iterative server */
void startServer()
{
    int server_fd;
    int client_fd;

    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;

    socklen_t addr_size;

    server_fd = createSocket();

    bindServer(server_fd, &server_addr);

    startListening(server_fd);

    printf("========================================\n");
    printf("       TCP FILE SERVER ONLINE           \n");
    printf("       Listening on Port: %d            \n", PORT);
    printf("========================================\n");
    printf("Waiting for an incoming transfer request...\n");

    /* Iterative server */
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
        printf(" SUCCESS: Remote client connection linked\n");
        printf("========================================\n");

        handleClient(client_fd);

        close(client_fd);

        printf("Waiting for next client...\n");
    }

    close(server_fd);
}