#include "client.h"
#include <iostream>

using namespace std;

int main(int argc, char** argv) {

	if (argc != 3){
		printf("Pemakaian: ./client <server ip> <nilai n>\n");
	}

	Client c;

	c.openSocket();
	c.getHost(argv[1]);
	c.connectToServer();
	c.writeMessage(argv[2]);
	c.readReply();
	c.closeClient();

	return 0;
}