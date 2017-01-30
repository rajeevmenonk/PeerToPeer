#include <iostream>
#include <vector>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>

using namespace std;
int main()
{
   int sockDesc;
   struct sockaddr_in serverAddr;
   bzero((char *)&serverAddr, sizeof(sockaddr_in));

   struct hostent *server;
   server = gethostbyname("127.0.0.1");

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
   //TODO - strcpy
   bcopy((char *)server->h_addr, (char *)&serverAddr.sin_addr.s_addr, server->h_length);
   serverAddr.sin_port = htons(12345);

   if (connect(sockDesc, (struct sockaddr *)&serverAddr,sizeof(serverAddr)) < 0) 
   {
       cout << "Unable to connect to socket\n";
       exit(0);
   }

   char buffer[100] = "TEST MEssage";
   int ret;
   ret = write(sockDesc, buffer, strlen(buffer)+1);
   close(sockDesc);
}
