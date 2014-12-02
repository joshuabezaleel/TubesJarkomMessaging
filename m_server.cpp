#include "server.h"
#include <iostream>

using namespace std;

int main() {

	Server s;

	s.openSocket();
	s.changeOption();
	s.setAddress();
	s.bindAddress();
	s.listenClient(5);

	while(1) {
		
		s.acceptConnection();
		s.readMessage();
		s.calculateN();
		s.closeClient();
	}
	s.closeServer();

	return 0;
}