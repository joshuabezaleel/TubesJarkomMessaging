#ifndef _CLIENT_H
#define _CLIENT_H

#include <cstdio>
#include <sys/types.h>   // tipe data penting untuk sys/socket.h dan netinet/in.h
#include <netinet/in.h>  // fungsi dan struct internet address
#include <sys/socket.h>  // fungsi dan struct socket API
#include <netdb.h>       // lookup domain/DNS hostname
#include <unistd.h>
#include <cstdlib>
#include <errno.h>
#include <cstring>

class Client {
public:
	Client();
	Client(int number);
	~Client();

	void openSocket();
	void getHost(char* number);
	void connectToServer();
	void writeMessage(char* message);
	void readReply();
	void closeClient();

private:
	int sock, port, len; char buffer[10];
	struct sockaddr_in serv_addr;
	struct hostent *server;
};

#endif