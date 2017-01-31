#include <iostream>
#include <vector>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <pthread.h>
#include <cstdlib>
#include <cstdio>

using namespace std;

// This function is invoked when the server is sending all the connected
// clients. When the server is sending the connected clients, it would first
// send the number of connected clients.
void printConnectedServers (int sockDesc)
{
   int servers;
   int clientId;
   read(sockDesc, &servers, sizeof(int));
   servers = htonl(servers);
   cout << "Number of available clients: " << servers << endl;
   while(servers != 0)
   {
        read(sockDesc, &clientId, sizeof(int));
        clientId = htonl(clientId);
        cout << "Client Id: " << clientId << endl;
        servers--;
   }
}

// This function is used by the read thread. This function prints all the data
// that is being send by the server.
void *readFun (void *args)
{
    int sockDesc = *(int *)args;
    char buffer[100];
    int senderId;
    while(1)
    {
        read(sockDesc, &senderId, sizeof(int));
        senderId = ntohl(senderId);

        // If the sender Id is -1, it means that the server is sending all the 
        // connected clients.
        if (senderId == -1)
        {
            printConnectedServers(sockDesc);
        }
        else
        {
            read(sockDesc, buffer, 100);

            cout << "##################### Message from sender: " << 
                    senderId << endl;
            printf(" ##################### Message: %s\n", buffer);
        }
    }
}

// This function is used by the write thread. This function sends the data to 
// the server.
void *writeFun(void *args)
{
    int sockDesc = *(int *)args;
    int clientId = 0;
    char buffer[100];
    while(1)
    {
        cout << "Enter Client to initiate chat";
        cout <<"(Enter -1 to get all connected clients):\n";
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
   if (argc != 2)
   {
       cout << "This program needs one argument\n";
       cout << "1. The server IP\n";
       cout << "Eg: ./client 127.0.0.1 \n";
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
   bcopy((char *)server->h_addr, 
         (char *)&serverAddr.sin_addr.s_addr, 
         server->h_length);
   serverAddr.sin_port = htons(12344);

   if (connect(sockDesc, 
               (struct sockaddr *)&serverAddr,
               sizeof(serverAddr)) < 0) 
   {
       cout << "Unable to connect to socket\n";
       exit(0);
   }

   char name[100] = "testName";
   write(sockDesc, name, strlen(name)+1);
   
   printConnectedServers(sockDesc);

   pthread_t readId, writeId;
   pthread_create(&readId, NULL, readFun, (void *)&sockDesc);
   pthread_create(&writeId, NULL, writeFun, (void *)&sockDesc);

   pthread_join(readId, NULL);
   pthread_join(writeId, NULL);
}
