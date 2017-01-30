#include <iostream>
#include <vector>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define SERVER_PORT 12345
#define MAX_CLIENTS 10

using namespace std;
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

    vector <int> clientSocks;
    int clientSock;
    struct sockaddr_in clientAddr;
    char buffer[100];
    socklen_t sizeOfSockAddr = sizeof(sockaddr_in);
    int size;
    while(1)
    {
        size = 0;
        clientSock = accept(sockDesc, (struct sockaddr *)&clientAddr, 
                   &sizeOfSockAddr);
        cout << "Client accepted\n";
        if (clientSock < 0)
        {
            cout << "Error on accepting a connection from client\n";
            continue;
        }
        size = read(clientSock, buffer + size, 100);
        printf("%s\n", buffer);
    }
}
