#include "server.h"
#define MAXHOSTNAME 256

using namespace std;

Server::Server() {
	port = 9000;
}

Server::Server(int number) {
	port = number;
}

Server::~Server() {

}

void Server::openSocket() {

	if((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP)) < 0){
		close(sock);
		printf("Cannot open socket\n");
		exit(1);
	}
	else {
		printf("Socket has opened\n");
	}
}

void Server::changeOption() {
	int yes = 1;
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
		perror("Cannot set option\n");
		exit(1);
	}
	else {
		printf("Option changed\n");
	}
}

void Server::setAddress() {
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY; // semua address pada network interface
	serv_addr.sin_port = htons(port); // port number
}

void Server::bindAddress() {
	if (bind(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
		close(sock);
		printf("Cannot bind socket\n");
		exit(1);
	}
	else {
		printf("Socket binding success\n");
	}
}

void Server::listenClient(int connection) {
	listen(sock, connection);
	printf("Listening on %d client\n", connection);
}

void Server::acceptConnection() {
	clilen = sizeof(cli_addr);
	client_sock = accept(sock, (struct sockaddr *) &cli_addr, &clilen);
}

void Server::readMessage() {
	// kosongkan buffer
	bzero(buffer, 10);
	// baca dari klien
	len = read(client_sock, buffer, 10);
}

void Server::calculateN() {
	int n;
	if (len >= 0){ // jika pembacaan berhasil
			n = atoi(buffer); // convert ke integer
			printf("Recv : %d\n",n);
			
			sprintf(buffer, "%d", n + 1); // convert n + 1 ke char*
			
			write(client_sock,buffer,10); // tulis ke klien
		}
}

void Server::closeServer() {
	close(sock);
}

void Server::closeClient() {
	close(client_sock);
}

// void Server::createThread() {

// 	 if(pthread_create( &thread_id , NULL , connectionHandler , (void*) &client_sock) < 0)
// 	{
// 		perror("could not create thread");
// 		exit(1);
// 	} 

// 	puts("Handler assign");
// }

// void *connectionHandler(void *socket_desc) {

// 	int sock = *(int*)socket_desc;
// 	int read_size;
// 	char *message , client_message[2000];

// 	//send message to client
// 	message = "Greetings! I am your connection handler\n";
// 	write(sock , message , strlen(message));
// 	message = "Now type something and i shall repeat what you type \n";
// 	write(sock , message , strlen(message));

// 	//receive message from client
// 	 while( (read_size = recv(sock , client_message , 2000 , 0)) > 0 )
// 	{
// 		//end of string marker
// 		client_message[read_size] = '\0';
// 		//Send the message back to client
// 		write(sock , client_message , strlen(client_message));
// 		//clear the message buffer
// 		memset(client_message, 0, 2000);
// 	}   

// 	 if(read_size == 0)
// 	{
// 		puts("Client disconnected");
// 		fflush(stdout);
// 	}
// 	else if(read_size == -1)
// 	{
// 		perror("recv failed");
// 	}
// 	return 0;
// }
