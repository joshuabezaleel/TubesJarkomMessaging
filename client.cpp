#include "client.h"

#include <cstdio>
#include <sys/types.h>   // tipe data penting untuk sys/socket.h dan netinet/in.h
#include <netinet/in.h>  // fungsi dan struct internet address
#include <sys/socket.h>  // fungsi dan struct socket API
#include <netdb.h>       // lookup domain/DNS hostname
#include <unistd.h>
#include <cstdlib>
#include <errno.h>
#include <cstring>


using namespace std;

Client::Client() {
	port = 9000;
}

Client::Client(int number) {
	port = number;
}

Client::~Client() {

}

void Client::openSocket() {

	if((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP)) < 0){
		close(sock);
		printf("Cannot open socket\n");
		exit(1);
	}
}

void Client::getHost(char* number) {

	server = gethostbyname(number);
	if (server == NULL) {
		fprintf(stderr,"Host not found\n");
		exit(1);
	}
	
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
	serv_addr.sin_port = htons(port);
}

void Client::connectToServer() {

	if (connect(sock,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
		exit(1);
	}
}

void Client::writeMessage(char* message) {

	// copy n ke buffer
	bcopy(message, buffer, 10);

	// tulis ke server
	len = write(sock,buffer,10);
}

void Client::readReply() {

	if (len >= 0){
		// baca balasan server
		len = read(sock,buffer,10);
		if (len >= 0){
			printf("%s\n", buffer);
		}
	}
}

void Client::closeClient() {

	close(sock);
}