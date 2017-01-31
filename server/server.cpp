#include <iostream>
#include <vector>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <map>
#include <cstdlib>

#define SERVER_PORT 12344
#define MAX_CLIENTS 10

using namespace std;

map < int, string > clientSocks;

// The following function sends all the connected clients to the 
// client that request the information
void sendAllClients(int clientSock)
{
    // Send client information to the child.
    //char serverName[100];
    int clientId;
    int mapSize = clientSocks.size();
    mapSize = htonl(mapSize);
    write(clientSock, &mapSize, sizeof(int));
    for ( map<int, string>::iterator iter = clientSocks.begin();
          iter != clientSocks.end();
          ++iter)
    {
        clientId = htonl(iter->first);
        write(clientSock, &clientId, sizeof(int));
    }
}

// This function is called to handle each client individually
void *handleClient (void *args)
{
    int clientSock = *(int *)args;
    char name[100];
    read(clientSock, name , 100);
    clientSocks[clientSock] = string(name);
    
    int clientId;

    sendAllClients(clientSock);

    char message[100];
    int size;
    int netOrderClientSock = htonl(clientSock);

    while(1)
    {
        bzero(message, 100);
        clientId = 0;

        read(clientSock, &clientId, sizeof(int));
        clientId = ntohl(clientId);

        if(clientId == -1)
        {
            clientId = htonl(clientId);
            write(clientSock, &clientId, sizeof(int));
            sendAllClients(clientSock);
        }
        else if (clientSocks.find(clientId) != clientSocks.end())
        {
            size = read(clientSock, &message, 100);
            
            write(clientId, &netOrderClientSock, sizeof(int));
            write(clientId, message, size+1);
        }
    }
    cout << "end " << clientSock << endl;
    free(&clientSock);
}

int main()
{
    int sockDesc;
    int port = SERVER_PORT;
    
    sockDesc = socket(AF_INET, SOCK_STREAM, 0);
    if (sockDesc < 0)
    {
        cout << "Error Opening Socket\n";
        exit(0);
    }

    struct sockaddr_in serverAddr;
    bzero((char *)&serverAddr, sizeof(sockaddr_in));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(sockDesc, 
             (struct sockaddr *) &serverAddr, 
             sizeof(sockaddr_in)) < 0)
    {
        cout << "Error Binding Socket\n";
        exit(0);
    }
    listen(sockDesc, MAX_CLIENTS);

    struct sockaddr_in clientAddr;
    socklen_t sizeOfSockAddr = sizeof(sockaddr_in);
    pthread_t threadId;

    while(1)
    {
        int *clientSock = new int;
        *clientSock = accept(sockDesc, (struct sockaddr *)&clientAddr, 
                   &sizeOfSockAddr);
        if (clientSock < 0)
        {
            cout << "Error on accepting a connection from client\n";
            continue;
        }
        pthread_create(&threadId, NULL, handleClient, (void *)clientSock);
    }
}
