#client file
all: client server

client: ./client/client.cpp
	g++ -g -Wall -o client.o ./client/client.cpp -pthread

server: ./server/server.cpp
	g++ -g -Wall -o server.o ./server/server.cpp -pthread
	
clean: 
	rm client.o server.o
