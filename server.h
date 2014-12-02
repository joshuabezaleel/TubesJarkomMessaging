#ifndef _SERVER_H
#define _SERVER_H

#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <unistd.h>
#include <errno.h>

class Server
{
public:
	Server();
	Server(int port);
	~Server();

	void openSocket();
	void changeOption();
	void setAddress();
	void bindAddress();
	void listenClient(int connection);
	void acceptConnection();
	void readMessage();
	void calculateN();
	void closeServer();
	void closeClient();

private:
	int port, sock, client_sock, len;
	char buffer[10];
	socklen_t clilen;
	struct sockaddr_in serv_addr, cli_addr;
	pthread_t thread_id;
};

#endif

