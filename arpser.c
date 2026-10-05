#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8082
#define MAX 100

struct ARPEntry
{
    char ip[20];
    char mac[20];
};

struct ARPEntry table[10] =
{
    {"192.168.1.1","AA:BB:CC:DD:EE:01"},
    {"192.168.1.2","AA:BB:CC:DD:EE:02"},
    {"192.168.1.3","AA:BB:CC:DD:EE:03"}
};

int n = 3;

int createSocket()
{
    int s = socket(AF_INET, SOCK_STREAM, 0);

    if(s < 0)
    {
        perror("Socket Creation Failed");
        exit(1);
    }

    return s;
}

void setupServer(int s, struct sockaddr_in *server)
{
    server->sin_family = AF_INET;
    server->sin_addr.s_addr = INADDR_ANY;
    server->sin_port = htons(PORT);

    // Allow quick port reuse to prevent "Address already in use" errors
    int opt = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if(bind(s,(struct sockaddr *)server,sizeof(*server)) < 0)
    {
        perror("Bind Failed");
        exit(1);
    }

    listen(s,5);
}

void processClient(int client)
{
    char ip[MAX], mac[MAX], response[MAX];
    int i;

    recv(client,ip,MAX-1,0);

    ip[strcspn(ip,"\n")] = '\0';

    printf("\n----------------------------------------\n");
    printf(" Processing Request for IP: %s\n", ip);
    printf("----------------------------------------\n");

    for(i=0;i<n;i++)
    {
        if(strcmp(ip,table[i].ip)==0)
        {
            strcpy(response,table[i].mac);
            send(client,response,strlen(response)+1,0);

            printf(" Status   : SUCCESS\n");
            printf(" Mapping  : %s -> %s\n",ip,response);
            printf("----------------------------------------\n");
            return;
        }
    }

    /* IP not found */
    send(client,"NOT_FOUND",10,0);
    printf(" Status   : NOT FOUND (Requesting Update)\n");

    recv(client,mac,MAX-1,0);
    mac[strcspn(mac,"\n")] = '\0';

    strcpy(table[n].ip,ip);
    strcpy(table[n].mac,mac);
    n++;

    send(client,"ADDED",6,0);

    printf(" Action   : New Entry Cached\n");
    printf(" Mapping  : %s -> %s\n",ip,mac);
    printf("----------------------------------------\n");
}

void startServer()
{
    int server,client;
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);

    server = createSocket();

    setupServer(server,&addr);

    printf("========================================\n");
    printf("     ARP TCP SERVER INITIALIZED         \n");
    printf("     Listening on Port: %d              \n", PORT);
    printf("========================================\n");

    while(1)
    {
        client = accept(server,
                        (struct sockaddr *)&addr,
                        &len);

        processClient(client);

        close(client);
    }
}

int main()
{
    startServer();
    return 0;
}
