#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

void *receive(void *sck);

int main(){
	// SERVER SETTINGS
	int clientSocket = socket(PF_INET, SOCK_STREAM, 0);
	struct sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(3205);
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if(connect(clientSocket, (struct sockaddr*) &serverAddr, sizeof(serverAddr)) == -1) 
		return 0;

	printf("Connected.\n");
  

	// INITIATIONS, THREADS, USERNAME
	pthread_t c_thread;
	pthread_create(&c_thread, NULL, receive, (void *) &clientSocket);
  	char phone[15];
  	printf("Enter your phone: ");
  	scanf("%s", phone);
	send(clientSocket,phone,15,0);
	
	// LET THE INPUT FLOW
	while(1){

		char buffer[1024];
		scanf("%s",buffer);

		if(strcmp(buffer, "-send") == 0){
			
			send(clientSocket,buffer,1024,0);  
			// THE SECOND INPUT FOR AN ADDRESS, TO WHOM?
			scanf("%s",buffer);
			send(clientSocket,buffer,1024,0);	
		}
		if(strcmp(buffer, "-join") == 0){
			
			send(clientSocket,buffer,1024,0);
			// YOU GOT A GROUP
			scanf("%s",buffer);
			send(clientSocket,buffer,1024,0);
            
            char password[15];
            // NEED PASSWORD TO JOIN
			printf("Password: \n");
			scanf("%s", password);
			send(clientSocket,password,1024,0);
   
		}
		
		if(strcmp(buffer,"-gcreate") == 0){
			// CREATE GROUP
			send(clientSocket,buffer,1024,0);
			scanf("%s",buffer);
			send(clientSocket,buffer,1024,0);
			// SET PASSWORD FOR IT
			char password[15];
  			printf("Password \n");
 			scanf("%s", password);
  			send(clientSocket,password,15,0);
			
		}
		if(strcmp(buffer,"-exit") == 0){
			// EXIT FROM THAT GROUP
			send(clientSocket,buffer,1024,0);
			// WHICH GROUP?
			scanf("%s",buffer);
			send(clientSocket,buffer,1024,0);
			
		}
		// USER INFO
		if(strcmp(buffer,"-whoami") == 0){
			send(clientSocket,buffer,1024,0);
			
		}
		if(strcmp(buffer,"-end") == 0){
			// TERMINATE
			send(clientSocket,buffer,1024,0);
			exit(0);

		}
		
	}

}

// DISPLAY
void *receive(void *sck){
	int sockID = *((int *) sck);
	
	while(1){
		char buffer[1024];
		int input = recv(sockID, buffer, 1024, 0);
		printf("%s\n",buffer);
	}
}
