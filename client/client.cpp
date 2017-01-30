#include <iostream>
#include <vector>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <pthread.h>

using namespace std;

void *readFun (void *args)
{
    int sockDesc = *(int *)args;
    char buffer[100];
    int ret;
    int senderId;
    while(1)
    {
        read(sockDesc, &senderId, sizeof(int));
        senderId = htonl(senderId);
        read(sockDesc, buffer, 100);
        printf("##################### Message from sender:%d %s\n", senderId, buffer);
    }
}

void *writeFun(void *args)
{
    int sockDesc = *(int *)args;
    int clientId = 0;
    char buffer[100];
    while(1)
    {
        cout << "Enter Client to initiate chat:\n";
        cin >> clientId;
        clientId = htonl(clientId);
        cout << "Enter Message:\n";
        cin >> buffer;
        write(sockDesc, &clientId, sizeof(int));
        write(sockDesc, buffer, strlen(buffer)+1);
    }
}

int main(int argc, char *argv[])
{
   if (argc != 3)
   {
       cout << "This program needs two arguments\n";
       cout << "1. The server IP\n";
       cout << "2. The client Name\n";
       cout << "Eg: ./client 127.0.0.1 client1\n";
       exit(0);
   }

   int sockDesc;
   struct sockaddr_in serverAddr;
   bzero((char *)&serverAddr, sizeof(sockaddr_in));

   struct hostent *server;
   server = gethostbyname(argv[1]);

   if (server == NULL)
   {
       cout << "Unable to get the server\n";
       exit(0);
   }

   sockDesc = socket(AF_INET, SOCK_STREAM, 0);
   if (sockDesc < 0)
   {
       cout << "Unable to create socket\n";
       exit(0);
   }

   serverAddr.sin_family = AF_INET;
   bcopy((char *)server->h_addr, (char *)&serverAddr.sin_addr.s_addr, server->h_length);
   serverAddr.sin_port = htons(12345);

   if (connect(sockDesc, (struct sockaddr *)&serverAddr,sizeof(serverAddr)) < 0) 
   {
       cout << "Unable to connect to socket\n";
       exit(0);
   }

   char name[100] = "Client1";
   int ret;
   ret = write(sockDesc, argv[2], strlen(argv[2])+1);
   
   int servers;
   int clientId;
   char buffer[100];
   ret = read(sockDesc, &servers, sizeof(int));

   servers = htonl(servers);
   if (servers == 0)
   {
       cout << "No otherr client";
   }

   int size = 0;
   while(servers != 0)
   {
        bzero(buffer, 100);
        read(sockDesc, &clientId, sizeof(int));
        clientId = htonl(clientId);
        
        read(sockDesc, buffer, 100);
        printf("Client Id: %d, Client Name: %s\n", clientId, buffer);
        servers--;
   }

   pthread_t readId, writeId;
   pthread_create(&readId, NULL, readFun, (void *)&sockDesc);
   pthread_create(&writeId, NULL, writeFun, (void *)&sockDesc);

   pthread_join(readId, NULL);
   pthread_join(writeId, NULL);

   //close(sockDesc);
}
