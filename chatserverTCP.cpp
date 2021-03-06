#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <iostream>
#include <netdb.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <pthread.h>

#include <unistd.h>

#define MAXTHREADS 3

#define KILO 1024
#define BUFF_LENGTH 1000
#define PROTO_PORT 60000
#define QLEN 1

#define MAX_CONTACTS 3

using namespace std;

int contacts = 0;
pthread_t tid[MAXTHREADS];	
int active_socket[MAXTHREADS];
int thread_retval = 0;
int sd;
int endloop;
int pending_num; //number of pending message
int numSize;
time_t rawtime;
FILE *temp;

typedef struct{
	char contactname[256];
	int contactsd;
}contact;

typedef struct{
	char sender[256];
	char receiver[256];
	char content[256];
	int status;	
}text;

text msg;

/* Buat user */
typedef struct{
	char username[BUFF_LENGTH];
	char password[BUFF_LENGTH];
	int status;
}dataUser;

dataUser listUser[50];
contact onlinecontacts[MAX_CONTACTS];

typedef struct group{
	char groupname[256];
	contact member[10];
}group;

// fungsi buat user-user an
void readFile();
bool logUser(char* user, char* pass);
bool checkUser(char* user);
void writeUser(char* user, char* pass);


void chat(int sd2){
	int n,i, served = 0;	
	int login = 0;
	printf("CHAAAAAAAAAAAAT\n");
	
	char clientname[BUFF_LENGTH];
	char *var;
	char message[BUFF_LENGTH];
	char inbuf[BUFF_LENGTH];
	char outbuf[BUFF_LENGTH];
	char *timestamp;
	
	char user[BUFF_LENGTH];
	char pass[BUFF_LENGTH];
	
	timestamp = (char*) malloc(1000);
	while(served == 0){

		for(i = 0; i < BUFF_LENGTH; i++){
			inbuf[i] = 0;
			outbuf[i] = 0;
			user[i] = 0;
			pass[i] = 0;
		}

		time (&rawtime);
		//read message from client
		n = read(sd2, inbuf, sizeof(inbuf));
		
		if(!strcmp(inbuf, "QUIT")){

			//logout 
			sprintf(outbuf, "QUIT");
			write(sd2, outbuf, sizeof(outbuf));			
			served = 1;
		}else{
			//login sebelum messaging
			if(login==0){
				if(!strcmp(inbuf, "LOGIN")){
					n = read(sd2, inbuf, sizeof(inbuf));
					strcpy(user, inbuf);
					printf("Nama : %s\n", user);
					
					n = read(sd2, inbuf, sizeof(inbuf));
					strcpy(pass, inbuf);
					printf("Password : %s\n", pass);
					
					if(logUser(user, pass)){
						sprintf(outbuf, "SUCCESS_LOGIN");
						write(sd2, outbuf, sizeof(outbuf));
						login = 1;
					}else{		// false
						sprintf(outbuf, "INVALID_LOGIN");
						write(sd2, outbuf, sizeof(outbuf));
					}
				}
				else if(!strcmp(inbuf, "SIGNUP")){
					n = read(sd2, inbuf, sizeof(inbuf));
					strcpy(user, inbuf);
					printf("Nama : %s\n", user);
					
					n = read(sd2, inbuf, sizeof(inbuf));
					strcpy(pass, inbuf);
					printf("Password : %s\n", pass);
					
					if(!checkUser(user)){
						sprintf(outbuf, "SUCCESS_SIGNUP");
						write(sd2, outbuf, sizeof(outbuf));
						writeUser(user, pass);
					}else{
						sprintf(outbuf, "INVALID_SIGNUP");
						write(sd2, outbuf, sizeof(outbuf));
					}
					
				}
				else{
					sprintf(outbuf, "LOGIN");
					write(sd2, outbuf, sizeof(outbuf));
				}
			}
			else{
				if(!strcmp(inbuf, "LOGOUT")){
					sprintf(outbuf, "LOGOUT");
					write(sd2, outbuf, sizeof(outbuf));
					login = 0;
				}
				else{
					//load temp storage
					text pending[20];
					char* pend_msg;
					pend_msg = (char*) malloc(1000);
					fscanf(temp, "%s", pend_msg);
					printf("%s\n", pend_msg);

					//interpret message
					if(!strncmp(inbuf, "<", 1)){
						var = strstr(inbuf, "<");
						i = 0;		
						var++;	
						while(*var != '>'){
							clientname[i] = *var;
							msg.receiver[i] = *var;
							printf("%c", *var);				
							var++;
							i++;
						}
						clientname[i] = '\0';
						msg.receiver[i] = '\0';
						
						var = strstr(inbuf, ">");
						i = 0;		
						var++;	
						while(*var != '\0'){
							message[i] = *var;
							msg.content[i] = *var;
							printf("%c", *var);				
							var++;
							i++;
						}
						message[i] = '\0';
						msg.content[i] = '\0';
						pending_num++;

						//menyimpan temp message
						fprintf(temp, "%s %s\n", msg.receiver, msg.content);
						printf("\nMessage [%s] is for [%s]\n\n", message, clientname);
					
					   // ensures the memory is an empty string
							strcat(timestamp,ctime(&rawtime));
							strcat(timestamp,message);
						

						for(i = 0; onlinecontacts[i].contactsd != sd2; i++);
						sprintf(outbuf, "<%s> %s", onlinecontacts[i].contactname, timestamp);

						//strcpy(outbuf, message);

						//write personal message
						i = 0;
						while(strcmp(onlinecontacts[i].contactname, clientname) && i<MAX_CONTACTS)
						{
							i++;
						}
						write(onlinecontacts[i].contactsd, outbuf, sizeof(outbuf));

					}else{
						for(i = 0; onlinecontacts[i].contactsd != sd2; i++);

							strcat(timestamp,ctime(&rawtime));
							strcat(timestamp,inbuf);	

						sprintf(message, "<%s> %s", onlinecontacts[i].contactname, timestamp);
						strcpy(outbuf, message);

						//broadcast message	
						for(i = 0; i < contacts; i++){
							if(onlinecontacts[i].contactsd != sd2)
								write(onlinecontacts[i].contactsd, outbuf, sizeof(outbuf));
							
						}
					}
				}
			}
		}
	}
	free(timestamp);
	printf("\nchat has finished\n");
	
}

void interrupt_handler(int sig){
	endloop = 1;
	close(sd);
	printf("Interrupt received: shutting down server!\n");
	return;
}
	
void *manage_connection(void *sdp) {

	char inbuf[BUFF_LENGTH];       /* buffer for incoming data  */
	char outbuf[BUFF_LENGTH];	/* buffer for outgoing data  */
	
	int i;			
	int sd2 = *((int *)sdp);
	int j = ((int *)sdp)-active_socket;	/* use pointer arithmetic to get this thread's index in array */
	//int thiscontact = contacts;
	for(i = 0; i < BUFF_LENGTH; i++){
		inbuf[i] = 0;
		outbuf[i] = 0;
	}

	//printf("\ndentro thread contacts [%d]\n\n", contacts);

	for(i = 0; i < contacts; i++){
	//	printf(" dentro for thread%s - %d\n\n", onlinecontacts[i].contactname, onlinecontacts[i].contactsd);

		//assign online client
		sprintf(outbuf, "[%d]: [%s]\n", i, onlinecontacts[i].contactname);
		//printf("[%s]\n\n", buffer);
		write(sd2, outbuf, sizeof(outbuf));
	}
		
	sprintf(outbuf, "END");
	write(sd2, outbuf, sizeof(outbuf));

	printf("-(IN THREAD)- sent online contacts\n");

	printf("-(IN THREAD)- simulazione di chat\n");
	

	chat(sd2);

	tid[j] = (pthread_t)-1; //è un cast...	/* free thread array entry */
	
	close(sd2);
	printf("-(IN THREAD)- chiuso sd2\n");
	contacts--;
	//onlinecontacts[thiscontact].contactname = "removed";
	//onlinecontacts[thiscontact].contactsd = 0;
	return &thread_retval;

}

int main(int argc, char** argv){
	
	if(argc < 2){
		printf("\nUsage: ./servertcp 'IP' ['port'] \nUse 127.0.0.1 as IP if you want to test program on localhost, port number is optional!\n\n");
	}

	struct sockaddr_in sad;
	struct sockaddr_in cad;
	socklen_t alen;
	
	contact newcontact;	
	
	int sd2, port, n, i, j = 0;
	char* var;
	char clientname[256];
	char busymsg[] = "BUSY";
	char buffer[BUFF_LENGTH];

	for(i=0;i<MAXTHREADS;i++) {
		tid[i] = (pthread_t)-1;
		active_socket[i] = -1;
	}

	for(i = 0; i < BUFF_LENGTH; i++){
		buffer[i] = 0;
	}
	
	//checking port
	if(argc == 3){
		port = atoi(argv[2]);
		while(port < 0 || port > 64 * KILO){
			printf("Bad port number, buond limits are (0,%d)\n\nEnter a new port number: ", 64 * KILO);
			scanf("%d", &port);
		}
	}else{
		port = PROTO_PORT;
	}


	//open temp file for message
	temp = fopen("temp_msg.txt","a+");

	//set address server
	memset((char*)&sad, 0, sizeof(sad));
	sad.sin_family = AF_INET;
	n = inet_aton(argv[1], &sad.sin_addr);
	sad.sin_port = htons((u_short)port);

	printf("Server IP address and service port: [%s]:[%d]\n", argv[1], port);
	printf("Server IP address and Service Port [%d]:[%d]\n\n", sad.sin_addr.s_addr, sad.sin_port);

	//open socket
	sd = socket(PF_INET, SOCK_STREAM, 0);
	if(sd < 0) {
		perror("Socket creation failed\n");
		exit(1);
	}
	printf("Socket created with sd: [%d]\n\n", sd);

	//bind socket address
	n = bind(sd, (struct sockaddr *)&sad, sizeof(sad));
	if(n == -1){
		perror("Error in Bind\n");
		exit(1);
	}

	n = listen(sd, QLEN);
	if(n < 0){
		perror("Listen failed\n");
		exit(1);
	}
	
	signal(SIGINT, interrupt_handler);

	printf("Server in the service loop\n");
	
	// read file user
	readFile();

	while(!endloop){

		alen = sizeof(cad);
	
		printf("Server is waiting for a Client to serve...\n");
		
		//accept connection
		sd2 = accept(sd, (struct sockaddr *)&cad, &alen);
		if (sd2 < 0) {
			if(endloop) break;
			perror("Accept failed\n");
			exit(1);
		}	
	
		if(contacts < MAXTHREADS) {
			
			printf("Connection with Client: [%s]:[%hu] - sd:[%d]\n", inet_ntoa(cad.sin_addr), ntohs(cad.sin_port), sd2);
			printf("This server is serving [%d] client%c\n", contacts + 1, (contacts > 0) ? 's' : ' ');

		//read from client
			n = read(sd2, buffer, sizeof(buffer));
			printf("Message from client: [%s]\n", buffer);
		
			var = strstr(buffer, "<");
			i = 0;		
			var++;	
			while(*var != '>'){
				clientname[i] = *var;
				printf("%c", *var);				
				var++;
				i++;
			}
			clientname[i] = '\0';
				
			printf("\nClient name is:  [%s]\n\n", clientname);
			
			//assign online client	
			strcpy(newcontact.contactname, clientname);
			newcontact.contactsd = sd2;
			onlinecontacts[contacts] = newcontact;

			//look for the first empty slot in thread array 
			for(i=0;tid[i]!=(pthread_t)-1;i++);
			
			 /*the use of different variables for storing socket ids
			 * avoids potential race conditions between the access
			 * to the value of &sd2 in the new started thread and
			 * the assignement by the connect() function call above.
			 * By Murphy's laws it my happen that the thread reads
			 * the variable pointed by its argument value after
			 * accept has stored a new value in sd2, thus loosing the
			 * previously opened socket.
			 */
			
		
			//printf(" prima if pthread %d\n\n", contacts);		
			active_socket[i] = sd2;

			//create thread
			if(pthread_create(&tid[i], NULL, manage_connection, &active_socket[i])!=0) {
				perror("Thread creation");
				tid[i] = (pthread_t)-1; // to be sure we don't have unknown values... cast
				continue;
			}
			contacts++;
		
		} else {  //too many threads 
			printf("Maximum threads active: closing connection\n");
			write(sd2, busymsg, strlen(busymsg)+1);
			close(sd2);
		}

		printf("nuovo thread attivato\n");

	}
	fclose(temp);
	printf("Server finished\n");
}

/** BACA FILE BUAT USER **/
void readFile(){
	FILE *fp;
    
    if ((fp = fopen("user.txt", "r")) == NULL) {
        printf("user.txt tidak bisa dibuka");
    } else {
		int i = 0;
		//rewind(fp);
		
		fscanf (fp, "%d", &numSize);		// baca baris pertama untuk jumlah user
		
		for(i=0; i<numSize; i++){
			fscanf (fp, "%s", listUser[i].username);
			fscanf (fp, "%s", listUser[i].password);
		}
		
		for(i=0; i<numSize; i++){
			printf ("%s\n", listUser[i].username);
			printf ("%s\n", listUser[i].password);
		}
		
		fclose(fp);

    }
}

/** MENULIS USER BARU KE FILE **/
void writeUser(char* user, char* pass){
	FILE *fp;
    
    if ((fp = fopen("user.txt", "w+")) == NULL) {
        printf("user.txt tidak bisa dibuka");
    } else {
		int i = 0;
		//rewind(fp);
		numSize = numSize + 1;
		strcpy(listUser[numSize-1].username, user);
		strcpy(listUser[numSize-1].password, pass);
		
		fprintf (fp, "%d\n", numSize);		// baca nulis pertama untuk jumlah user
		
		for(i=0; i<numSize; i++){
			fprintf (fp, "%s\n", listUser[i].username);
			fprintf (fp, "%s\n", listUser[i].password);
		}
		
		fclose(fp);
    }
    readFile();
}


void readTempMsg(){
	FILE *fp;
    
    if ((fp = fopen("temp_msg.txt", "r")) == NULL) {
        printf("user.txt tidak bisa dibuka");
    } else {
		int i = 0;
		//rewind(fp);
		
		fscanf (fp, "%d\n", &pending_num);		// baca baris pertama untuk jumlah user
		
		for(i=0; i<pending_num; i++){
			fscanf (fp, "%s\n", listUser[i].username);
			fscanf (fp, "%s\n", listUser[i].password);
		}
		
		for(i=0; i<pending_num; i++){
			printf ("%s\n", listUser[i].username);
			printf ("%s\n", listUser[i].password);
		}
		
		fclose(fp);

    }
}

/** LOGIN USER **/
bool logUser(char* user, char* pass){
	int i = 0;
	int j, k;
	bool ketemu = false;
	
	while((i<numSize) && !ketemu){
		if(!strcmp(user, listUser[i].username)){
			if(!strcmp(pass, listUser[i].password)){
				ketemu = true;
			}
			else{
				i++;
			}
		}
		else{
			i++;
		}
	}
	
	return ketemu;
}
/** MENGECEK USER APAKAH SUDAH ADA **/
bool checkUser(char* user){
	int i = 0;
	bool ada = false;
	
	while((i<numSize) && !ada){
		if(!strcmp(user, listUser[i].username)){
			ada = true;
		}
		else{
			i++;
		}
	}
	
	return ada;
}
