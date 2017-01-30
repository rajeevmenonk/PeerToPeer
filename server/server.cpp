#include <iostream>
#include <vector>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <map>

#define SERVER_PORT 12345
#define MAX_CLIENTS 10

using namespace std;

map < int, string > clientSocks;
pthread_mutex_t  mutex; 

void *threadFun (void *args)
{
    int clientSock = *(int *)args;
    char name[100];
    read(clientSock, name , 100);
    clientSocks[clientSock] = string(name);
    
    pthread_mutex_lock(&mutex);
    pthread_mutex_unlock(&mutex);
    int clientId;
    char clientName[100];
    int mapSize = clientSocks.size();
    cout << "Map size is " << mapSize << endl;
    mapSize = htonl(mapSize);
    write(clientSock, &mapSize, sizeof(int));

    // Send client information to the child.
    char serverName[100];
    for ( map<int, string>::iterator iter = clientSocks.begin();
          iter != clientSocks.end();
          ++iter)
    {
        clientId = iter->first;
        if (1 || clientId != clientSock)
        {
            clientId = htonl(clientId);
            write(clientSock, &clientId, sizeof(int));
            strcpy(serverName, (iter->second).c_str());
            write(clientSock, serverName, strlen(serverName));
        }
    }

    char message[100];
    int size;
    int netOrderClientSock = htonl(clientSock);

    while(1)
    {
        size = 0;
        bzero(message, 100);

        read(clientSock, &clientId, sizeof(int));
        clientId = htonl(clientId);
        size = read(clientSock, &message, 100-size);
        
        write(clientId, &netOrderClientSock, sizeof(int));
        write(clientId, message, size+1);
    }
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

    if (bind(sockDesc, (struct sockaddr *) &serverAddr, sizeof(sockaddr_in)) < 0)
    {
        cout << "Error Binding Socket\n";
        exit(0);
    }
    listen(sockDesc, MAX_CLIENTS);

    struct sockaddr_in clientAddr;
    socklen_t sizeOfSockAddr = sizeof(sockaddr_in);
    int size;
    pthread_t threadId;
    pthread_mutex_init(&mutex, NULL);

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
        pthread_create(&threadId, NULL, threadFun, (void *)clientSock);
    }
}
