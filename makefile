all:
	g++ -o server chatserverTCP.cpp -pthread
	g++ -o client chatclientTCP.cpp -pthread
	
runserver:
	./server
	
runclient:
	./client
