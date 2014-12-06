/*
 * chatclientTCP.c
 *
 *  Created on: Feb 2013
 *      Author: Matteo Ruggero Ronchi
 *
 *  Copyright 2013 Matteo Ruggero Ronchi
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> 
#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>

#define KILO 1024
#define BUFF_LENGTH 1000
#define PROTO_PORT 60000

using namespace std;
int sd;
int served = 0;
FILE *inbox;


//membaca kiriman server
void *manage_reading(void* number){
	
	int n, i;
	char inbuf[BUFF_LENGTH];	
	time_t rawtime;

	while(served == 0){

		for(i = 0; i < BUFF_LENGTH; i++){
			inbuf[i] = 0;
		}

		n = read(sd, inbuf, sizeof(inbuf));

		if(!strcmp(inbuf, "QUIT"))
			served = 1;
		else
			printf("Received message!!: %s\n", inbuf);

			time (&rawtime);
			//fprintf(inbox,"%s\n", ctime (&rawtime));
			//fprintf(inbox,"%s\n", ctime (&rawtime));
			fprintf(inbox,"%s\n",inbuf);		
	}	
}

void *manage_writing(void* number) {

	char outbuf[BUFF_LENGTH];
	int i;

	while(served == 0){
			
			for(i = 0; i < BUFF_LENGTH; i++){
				outbuf[i] = 0;
			}

			while(getchar() != '\n');		//secondo client in poi prima stampa a vuoto...
			scanf("%[^\n]s", outbuf);
			write(sd, outbuf, sizeof(outbuf));			
			if(!strcmp(outbuf, "QUIT"))
				served = 1;						
		}	
}

int main(int argc, char** argv){

	if(argc < 3){
		printf("\nUsage: ./clienttcp 'wanted client host name' 'IP'\nUse 127.0.0.1 as IP if you want to test program on localhost!\n\n");
	}

	struct sockaddr_in sad;
	socklen_t alen;

	int i, n, port, oc = 0;
	char inbuf[BUFF_LENGTH], outbuf[BUFF_LENGTH], outchar;
	char *hostname;
	pthread_t tid, tid2;

	for(i = 0; i < BUFF_LENGTH; i++){
		inbuf[i] = 0;
		outbuf[i] = 0;
	}

	//assign port number
	if(argc == 4){
		port = atoi(argv[3]);
		while(port < 0 || port > 64 * KILO){
			printf("Bad port number, buond limits are (0,%d)\n\nEnter a new port number: ", 64 * KILO);
			scanf("%d", &port);
		}
	}else{
		port = PROTO_PORT;
	}

	//buat adress server	
	memset((char*)&sad, 0, sizeof(sad));
	sad.sin_family = AF_INET;
	sad.sin_port = htons((u_short)port);
	n = inet_aton(argv[2], &sad.sin_addr);
	printf("\nServer to contact: [%s]:[%d]\n", argv[2], port);
	printf("Server to contact: [%d]:[%d]\n\n", sad.sin_addr.s_addr, sad.sin_port);

	hostname = argv[1];
	printf("Client's hostname is: [%s]\n\n", hostname);

	//create socket
	sd = socket(PF_INET, SOCK_STREAM, 0);
	
	printf("Connecting to [%d]:[%d]...\n\n", sad.sin_addr.s_addr, sad.sin_port);
	sleep(5);

	//initiate connection
	connect(sd, (struct sockaddr*)&sad, sizeof(sad));
	printf("Connected to Server: [%d]:[%d]\n", sad.sin_addr.s_addr, sad.sin_port);
	
	//write to server
	sprintf(outbuf, "HELLO I AM <%s>", hostname);
	write(sd, outbuf, sizeof(outbuf));
	
	//open file inbox
	char* client_inbox;
	
	client_inbox = (char*) malloc(1000);
	strcat(client_inbox,hostname);
	strcat(client_inbox,".txt");
	
	inbox = fopen(client_inbox,"a+");

	printf("waiting for Server status...\n\n");
	sleep(5);
		
	//read from server	
	n = read(sd, inbuf, sizeof(inbuf));
	if(strcmp(inbuf, "BUSY")){
		printf("Online Contacts:\n");
		while(strcmp(inbuf, "END")){
			printf("\t\t - %s\n", inbuf);
			n = read(sd, inbuf, sizeof(inbuf));
			oc++;
		}

		if(oc == 0){
reask:			printf("No contacts online, do you wish to close connection? (Y/N)\n");
			scanf("%c", &outchar);
			switch(outchar){
				case 'Y': goto out;
					  	
				case 'y': goto out;
					  
				case 'N': 
					  break;
				case 'n': 
					  break;
				default: 
					printf("wrong character pressed\n");
					while(getchar() != '\n');					
					goto reask;
					break;
			}		
		}

		printf("\t\t- <[contactname]:[message] to send a private message>\n\t\t- <[message] to send a public message>\n\t\t- <[QUIT] to exit>\n\n"); //controlla nuovi arrivi utenti
		
		int a;
		if(pthread_create(&tid, NULL, manage_reading, NULL)!=0) {
			perror("Thread creation");
		}

		if(pthread_create(&tid2, NULL, manage_writing, NULL)!=0) {
			perror("Thread creation");
		}		

		pthread_join(tid, NULL);
		while(served == 0){
			
			for(i = 0; i < BUFF_LENGTH; i++){
				outbuf[i] = 0;
			}

			while(getchar() != '\n');		//secondo client in poi prima stampa a vuoto...
			scanf("%[^\n]s", outbuf);
			write(sd, outbuf, sizeof(outbuf));			
			if(!strcmp(outbuf, "QUIT"))
				served = 1;						
		}
		
		//pthread_join(tid2, NULL);

	}else{
		printf("\nServer Busy, closing connection\n");
		close(sd);	
	}

out:	printf("Closing connection...\n");

	sleep(1);

	printf("Bye!\n");
	sleep(5);
	
	close(sd);
	free(client_inbox);
	fclose(inbox);
	printf("\n\nClient finished\n\n");
}
