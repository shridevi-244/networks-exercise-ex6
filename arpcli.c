#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8082
#define MAX 100

int createSocket()
{
    int s = socket(AF_INET,SOCK_STREAM,0);

    if(s < 0)
    {
        perror("Socket Creation Failed");
        exit(1);
    }

    return s;
}

void connectServer(int s, struct sockaddr_in *server)
{
    server->sin_family = AF_INET;
    server->sin_port = htons(PORT);
    server->sin_addr.s_addr = inet_addr("127.0.0.1");

    if(connect(s,(struct sockaddr *)server,
               sizeof(*server)) < 0)
    {
        perror("Connection Refused");
        exit(1);
    }
}

void requestARP(int s)
{
    char ip[MAX], mac[MAX], response[MAX];

    printf("========================================\n");
    printf("          ARP CLIENT UTILITY            \n");
    printf("========================================\n");
    printf("Enter target IP address : ");
    scanf("%19s",ip);
    printf("----------------------------------------\n");

    send(s,ip,strlen(ip)+1,0);

    recv(s,response,MAX-1,0);

    if(strcmp(response,"NOT_FOUND")==0)
    {
        printf("Result    : IP address target not resolved\n");
        printf("Action    : Enter missing physical address\n");
        printf("----------------------------------------\n");
        printf("Enter MAC address       : ");
        scanf("%19s",mac);
        printf("----------------------------------------\n");

        send(s,mac,strlen(mac)+1,0);

        recv(s,response,MAX-1,0);

        printf("========================================\n");
        printf("       NEW ARP RECORD COMMITTED         \n");
        printf("========================================\n");
        printf(" IP Protocol Address : %s\n",ip);
        printf(" MAC Hardware Address: %s\n",mac);
        printf("----------------------------------------\n");
    }
    else
    {
        printf("========================================\n");
        printf("       ARP RESOLUTION SUCCESSFUL        \n");
        printf("========================================\n");
        printf(" IP Protocol Address : %s\n",ip);
        printf(" MAC Hardware Address: %s\n",response);
        printf("----------------------------------------\n");
    }
}

void startClient()
{
    int s;
    struct sockaddr_in server;

    s = createSocket();

    connectServer(s,&server);

    requestARP(s);

    close(s);
}

int main()
{
    startClient();
    return 0;
}