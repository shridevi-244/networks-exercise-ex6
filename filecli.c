#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8081
#define BUFFER_SIZE 1024

int createSocket();
void setServerAddress(struct sockaddr_in *server_addr);
void connectServer(int sock, struct sockaddr_in *server_addr);
void requestFile(int sock);
void startClient();

int main()
{
    startClient();
    return 0;
}

/* Create TCP socket endpoints */
int createSocket()
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("Socket creation failed");
        exit(1);
    }
    return sock;
}

/* Form destination network profile structure */
void setServerAddress(struct sockaddr_in *server_addr)
{
    server_addr->sin_family = AF_INET;
    server_addr->sin_port = htons(PORT);
    server_addr->sin_addr.s_addr = inet_addr("127.0.0.1");
}

/* Establish active system connection links */
void connectServer(int sock, struct sockaddr_in *server_addr)
{
    if (connect(sock, (struct sockaddr *)server_addr, sizeof(*server_addr)) < 0)
    {
        printf("\n[ERROR] Connection failed! Server must be active first.\n\n");
        close(sock);
        exit(1);
    }
}

/* Trigger formatted file request dialogue loop */
void requestFile(int sock)
{
    char filename[BUFFER_SIZE];
    char buffer[BUFFER_SIZE];
    int bytes;

    printf("\n==================================================\n");
    printf("               TCP FILE TRANSFER CLIENT           \n");
    printf("==================================================\n");

    printf("Enter filename to request: ");
    scanf("%s", filename);
    printf("--------------------------------------------------\n");

    /* Forward clean filename parameters to server system */
    send(sock, filename, strlen(filename) + 1, 0);

    /* Grab status confirmations */
    memset(buffer, 0, BUFFER_SIZE);
    bytes = recv(sock, buffer, BUFFER_SIZE - 1, 0);

    if (bytes <= 0)
    {
        printf("Server disconnected.\n");
        printf("==================================================\n\n");
        return;
    }

    buffer[bytes] = '\0';

    if (strcmp(buffer, "File present") == 0)
    {
        printf("Status: FILE PRESENT (Receiving data...)\n");
        printf("==================================================\n");
        printf("[FILE CONTENTS]\n");
        printf("--------------------------------------------------\n");

        /* Stream out received blocks directly onto terminal display screen */
        while ((bytes = recv(sock, buffer, BUFFER_SIZE - 1, 0)) > 0)
        {
            buffer[bytes] = '\0';
            printf("%s", buffer);
        }

        printf("\n--------------------------------------------------\n");
        printf("End of File Reached - Transfer Successful.\n");
    }
    else
    {
        printf("Status: FILE NOT PRESENT ON SERVER\n");
    }

    printf("==================================================\n\n");
}

/* Handle execution orchestration routines */
void startClient()
{
    int sock;
    struct sockaddr_in server_addr;

    sock = createSocket();
    setServerAddress(&server_addr);
    connectServer(sock, &server_addr);

    printf("\nConnected to server successfully.\n");

    requestFile(sock);
    close(sock);
}
