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

#define MAX_GROUP 5
#define CLIENT_IN_GROUP 20
#define MAX_CLIENT 1000

struct client { 
    int ID;
    int sockID;
    struct sockaddr_in clientAddr;
    int room_check;
    int length;
    char phone[12]; 

};

struct group { 
    int ID;
    char group_name[20]; 
    struct client clients[CLIENT_IN_GROUP];
    int client_count; 
    char password[15]; 
};

// PROTOTYPES
void createGroup(int index, int clientSocket);
void join(int current_group, int index, int clientSocket);
void end(int index, int current_group);
void * network(void * netclient);

// INITIATIONS
struct client clients[MAX_CLIENT];
struct group groups[MAX_GROUP];
pthread_t c_threads[MAX_CLIENT];
struct client empty_client = {
    0
};
struct group empty_group = {
    0
};
int client_id = 0;
int group_id = 0;

int main() {
	// SERVER SETTINGS
    int serverSocket = socket(PF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(3205);
    serverAddr.sin_addr.s_addr = htons(INADDR_ANY);

    // BIND
    if (bind(serverSocket, (struct sockaddr * ) & serverAddr, sizeof(serverAddr)) == -1)
        return 0;
    // LISTEN
    if (listen(serverSocket, 1024) == -1)
        return 0;

    printf("CONNECTING... \n");
	
	
	// THREAD LOOP
    while (1) {       
        clients[client_id].sockID = accept(serverSocket, (struct sockaddr*) &clients[client_id].clientAddr, &clients[client_id].length);
		clients[client_id].ID = client_id;
		pthread_create(&c_threads[client_id], NULL, network, (void *) &clients[client_id]);
		client_id++; 
    }
	// JOIN THREADS
    for (int i = 0; i < client_id; i++)
        pthread_join(c_threads[i], NULL);

}
// GCREATE
void createGroup(int index, int clientSocket) {
    char str[1024];
    char display[100];
    int input = recv(clientSocket, str, 1024, 0);
	// GROUP SETTINGS
    groups[group_id].ID = group_id;
    strcpy(groups[group_id].group_name, str);
    groups[group_id].client_count = 0;
    groups[group_id].clients[0] = clients[index];
    clients[index].room_check = 1;
    groups[group_id].client_count++;
}
// JOIN
void join(int current_group, int index, int clientSocket) {
    groups[current_group].clients[groups[current_group].client_count] = clients[index];
    clients[index].room_check = 1;
    groups[current_group].client_count++;
}
// EXIT
void end(int index, int current_group) {
	// FIND THE CURRENT CLIENT
    int current_client = 0;
    for (int i = 0; i < groups[current_group].client_count; i++) {
        if (clients[index].ID == groups[current_group].clients[i].ID) {
            current_client = i;
            break;
        }
    }
    // RE-STRUCT THE GROUP SO THAT THE CLIENT WHO LEFT WON'T TAKE UNNECESSARY SPACE
    groups[current_group].clients[current_client] = empty_client; 
    for (int i = current_client; i < groups[current_group].client_count - current_client; i++) {
        if (i == groups[current_group].client_count - 1) {
            break;
        }
        groups[current_group].clients[i] = groups[current_group].clients[i + 1];
    }
	// THE CLIENT WHO'S LEFT IS ASSIGNED WITH AN EMPTY ONE.
    groups[current_group].clients[groups[current_group].client_count - 1] = empty_client; 
    groups[current_group].client_count--;
    clients[index].room_check = 0; 
	// FILL IN THE EMPTY SPACES ONE BY ONE
    if ((groups[current_group].client_count) == 0) { 
        groups[current_group] = empty_group;
        for (int j = current_group; j < group_id - current_group; j++) {
            if (j == group_id - 1) {
                break;
            }
            groups[current_group] = groups[current_group + 1];
        }

        groups[group_id] = empty_group;
        group_id--;
    }

}
// CLIENT FUNCTION TO PASS OVER THE THREAD
void * network(void * netclient) { 
    struct client * current_client = (struct client * ) netclient;
    char phone[15];
    int ID = current_client -> ID;
    int client_socket = current_client -> sockID;
    int input = recv(client_socket, phone, 15, 0);
    strcpy(current_client -> phone, phone);
    
    printf("%s has connected.\n", phone);

    while (1) {
		// KEEP RECEIVING INPUT
        char password[15];
        char buffer[1024];
        int input = recv(current_client -> sockID, buffer, 1024, 0);
        buffer[input] = '\0';
		
		// SEND MESSAGE
        if (strcmp(buffer, "-send") == 0) {
            if (clients[current_client -> ID].room_check == 1) {
                recv(current_client -> sockID, buffer, 1024, 0);
                char output[500];
                strcpy(output, phone);
                strcat(output, ": ");
                strcat(output, buffer);
                strcat(output, "\n");
				// FIND THE CURRENT GROUP
                int current_group = -1;
                for (int i = 0; i < group_id; i++) {
                    for (int j = 0; j < groups[i].client_count; j++) {
                        if (strcmp(groups[i].clients[j].phone, clients[current_client -> ID].phone) == 0) {
                            current_group = i;
                        }
                    }
                }
				// DISPLAY MESSAGE
                for (int i = 0; i < groups[current_group].client_count; i++) {
                    send(groups[current_group].clients[i].sockID, output, 500, 0);
                }
            } else {
                char tmp[40];
                strcpy(tmp, "You're not in a group.");
                send(clients[current_client -> ID].sockID, tmp, 40, 0);
            }
        }
		// CREATE GROUP W/ PASSWORD
        if (strcmp(buffer, "-gcreate") == 0) {
            if (clients[current_client -> ID].room_check != 1) {
                char output[200];
                createGroup(current_client -> ID, current_client -> sockID);
                char group_aux[20];
                strcpy(group_aux, groups[group_id].group_name);
                group_id++;
                strcpy(output, "Group ");
                strcat(output, group_aux);
                strcat(output, " is created.\n");
                recv(current_client -> sockID, password, 15, 0);
                
                strcpy(groups[group_id].password, password);
                send(clients[current_client -> ID].sockID, output, 200, 0);
            } else {
                char tmp[40];
                strcpy(tmp, "You're already in a group.");
                send(clients[current_client -> ID].sockID, tmp, 40, 0);
            }
        }
    
	// JOIN TO A GROUP
    if (strcmp(buffer, "-join") == 0) {
        char output[200];
        int current_group = -1;
        input = recv(current_client -> sockID, buffer, 1024, 0);
        buffer[input] = '\0';
        char temp_group[20];
        strcpy(temp_group, buffer);
        // IS THE CLIENT ALREADY IN A GROUP ?
        if ((clients[current_client -> ID].room_check) != 1) {
	
            for (int i = 0; i < group_id; i++) {
                if (strcmp(groups[i].group_name, temp_group) == 0) {
                    current_group = groups[i].ID;
                    break;
                }
            }
			// GROUP EXISTS
            if (current_group != -1) { 
                char password[15];
                input = recv(current_client -> sockID, password, 15, 0); 
                password[input] = '\0';
                if (strcmp(groups[current_group].password, password) == 0) { 
                    join(current_group, current_client -> ID, current_client -> sockID);
                    strcpy(output, "Client ");
                    strcat(output, clients[current_client -> ID].phone);
                    strcat(output, " has joined the group ");
                    strcat(output, groups[current_group].group_name);
                    strcat(output, ".\n");
                    for (int i = 0; i < groups[current_group].client_count; i++) {
                        send(groups[current_group].clients[i].sockID, output, 200, 0);
                    }
                    // FALSE PASS
                } else { 
                    char tmp[40];
                    strcpy(tmp, "You've entered a wrong password! \n");
                    send(clients[current_client -> ID].sockID, tmp, 40, 0);
                }
			// GROUP? NO.
            } else { 
                char tmp[40];
                strcpy(tmp, "There's no such group.");
                send(clients[current_client -> ID].sockID, tmp, 40, 0);
            }
            // CAN'T JOIN ANOTHER ONE
        } else { 
            char tmp[40];
            strcpy(tmp, "You're already in a group.");
            send(clients[current_client -> ID].sockID, tmp, 40, 0);
        }
    }
	// GET USER INFO
    if (strcmp(buffer, "-whoami") == 0) {
        char output[100];
        strcpy(output, "INFO: ");
        strcat(output, phone);
        strcat(output, "\n");
        send(clients[current_client -> ID].sockID, output, 100, 0);

    }
	// EXIT FROM A GROUP
    if (strcmp(buffer, "-exit") == 0) {
        char output[100];
        // CLIENT IS IN A GROUP
        if ((clients[current_client->ID].room_check) == 1) { 
            int group_check = 0;
            int current_group = -1;
            // GET THE GROUP NAME FROM THE USER
            recv(current_client -> sockID, buffer, 1024, 0);
            char temp_group[20];
            strcpy(temp_group, buffer);
			// FIND IT
            for (int i = 0; i < group_id; i++) {
                if (strcmp(groups[i].group_name, temp_group) == 0) {
                    current_group = groups[i].ID;
                    break;
                }
            }
			// THE GROUP EXISTS
            if (current_group != -1) { 
				// IS THE CLIENT IN THE GROUP?
                for (int i = 0; i < groups[current_group].client_count; i++) {
                    if (strcmp(groups[current_group].clients[i].phone, clients[current_client -> ID].phone) == 0) {
                        group_check = 1;
                    }
                }
                // YES.
                if (group_check == 1) { 
                    end(current_client -> ID, current_group);
                    char output[200];
                    strcpy(output, "Client ");
                    strcat(output, clients[current_client -> ID].phone);
                    strcat(output, " left the group ");
                    strcat(output, groups[current_group].group_name);
                    strcat(output, ".\n");
                    send(clients[current_client -> ID].sockID, output, 200, 0);
                    for (int i = 0; i < groups[current_group].client_count; i++) {
                        send(groups[current_group].clients[i].sockID, output, 200, 0);
                    }
                    // NO.
                } else { 
                    send(clients[current_client -> ID].sockID, "You are not in the group.", 25, 0);
                }
            } else { 
                char tmp[40];
                strcpy(tmp, "Group not found!");
                send(clients[current_client -> ID].sockID, tmp, 40, 0);
            }
        } else 
        {
            char tmp[40];
            strcpy(tmp, "You are not in a room");
            send(clients[current_client -> ID].sockID, tmp, 40, 0);
        }
    }
	}
	return NULL;
}



