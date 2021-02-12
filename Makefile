all: server client

server:
	g++ -pthread -o server server.cpp

client:
	g++ -pthread -o client client.cpp
