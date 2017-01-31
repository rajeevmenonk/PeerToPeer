#include <iostream>
#include <vector>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <pthread.h>

using namespace std;

void printConnectedServers (int sockDesc)
{
   int servers;
   int clientId;
   read(sockDesc, &servers, sizeof(int));
   servers = htonl(servers);
   int size = 0;
   char buffer[100];
   cout << "Number of available clients: " << servers << endl;
   while(servers != 0)
   {
        read(sockDesc, &clientId, sizeof(int));
        clientId = htonl(clientId);
        //bzero(buffer, 100);
        //size = 0;
        //do
        //{
        //   read(sockDesc, buffer+size, 100-size);
        //   printf("current val %s\n", buffer);
        //}
        //while(buffer[size-1] != '\0');

        //printf("Client Id: %d Client Name:%s\n", clientId, buffer);
        printf("Client Id: %d\n", clientId);
        servers--;
   }
}

void *readFun (void *args)
{
    int sockDesc = *(int *)args;
    char buffer[100];
    int ret;
    int senderId;
    while(1)
    {
        ret = read(sockDesc, &senderId, sizeof(int));
        senderId = ntohl(senderId);
        if (senderId == -1)
        {
            printConnectedServers(sockDesc);
        }
        else
        {
            ret = read(sockDesc, buffer, 100);

            printf("##################### Message from sender:%d \n", senderId);
            printf("##################### Message:%s \n", buffer);
        }
    }
}

void *writeFun(void *args)
{
    int sockDesc = *(int *)args;
    int clientId = 0;
    char buffer[100];
    while(1)
    {
        cout << "Enter Client to initiate chat(Enter -1 to get all connected clients):\n";
        cin >> clientId;

        if (clientId != -1)
        {
            clientId = htonl(clientId);
            write(sockDesc, &clientId, sizeof(int));
            cout << "Enter Message:\n";
            cin >> buffer;
            write(sockDesc, buffer, strlen(buffer)+1);
        }
        else
        {
            clientId = htonl(clientId);
            write(sockDesc, &clientId, sizeof(int));
        }
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
   serverAddr.sin_port = htons(12344);

   if (connect(sockDesc, (struct sockaddr *)&serverAddr,sizeof(serverAddr)) < 0) 
   {
       cout << "Unable to connect to socket\n";
       exit(0);
   }

   int ret;
   ret = write(sockDesc, argv[2], strlen(argv[2])+1);
   
   /*
   int servers;
   int clientId;
   ret = read(sockDesc, &servers, sizeof(int));
   servers = htonl(servers);
   int size = 0;
   char buffer[100];
   while(servers != 0)
   {
        read(sockDesc, &clientId, sizeof(int));
        clientId = htonl(clientId);
        bzero(buffer, 100);
        size = 0;
        do
           read(sockDesc, buffer+size, 100-size);
        while(buffer[size-1] != '\0');

        printf("Client Id: %d Client Name:%s\n", clientId, buffer);
        servers--;
   }
   */
   printConnectedServers(sockDesc);

   pthread_t readId, writeId;
   pthread_create(&readId, NULL, readFun, (void *)&sockDesc);
   pthread_create(&writeId, NULL, writeFun, (void *)&sockDesc);

   pthread_join(readId, NULL);
   pthread_join(writeId, NULL);

   //close(sockDesc);
}
