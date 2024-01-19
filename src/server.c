/* 
    ********************************************************************
    Odsek:          Elektrotehnika i racunarstvo
    Departman:      Racunarstvo i automatika
    Katedra:        Racunarska tehnika i racunarske komunikacije (RT-RK)
    Predmet:        Osnovi Racunarskih Mreza 1
    Godina studija: Treca (III)
    Skolska godina: 2021/22
    Semestar:       Zimski (V)
    
    Ime fajla:      server.c
    Opis:           TCP/IP server
    
    Platforma:      Raspberry Pi 2 - Model B
    OS:             Raspbian
    ********************************************************************
*/

#include <stdio.h>
#include <string.h>    // strlen
#include <sys/socket.h>
#include <arpa/inet.h> // inet_addr
#include <unistd.h>    // write

#include <pthread.h>
#include <stdlib.h> // for malloc

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT   27015

// Function to check if the file exists
int FileExists(char* fileName){
    FILE* fp;
    if(fp = fopen(fileName, "r")){
        //puts("Exists");
        fclose(fp);
        return 1;
    }
    return 0;
}

// Structure for HTTP request
typedef struct HttpRequest{
    char method[10];
    char path[100];
    char protocol[20];
} HttpRequest;

// Function to parse the HTTP request
int ParseRequest(char *request, HttpRequest *parsedRequest){
    if(sscanf(request, "%9s %99s %19s", parsedRequest->method, parsedRequest->path, parsedRequest->protocol) != 3){
        return 1;
    }
    
    memmove(parsedRequest->path, parsedRequest->path + 1, strlen(parsedRequest->path)); // removes the "/" from the path
   
    return 0;
}

// Structure for socket data
typedef struct sockData{
    int* socket;
} sockData;

// Function called by the worker thread
void* Worker(void* args){
    
    int readSize;
    char clientRequest[DEFAULT_BUFLEN];
    
    sockData* clientTmpSock = (sockData*)args;
    int* clientSock = clientTmpSock->socket;
    
    readSize = recv(*clientSock , clientRequest , DEFAULT_BUFLEN , 0);
    if(readSize == -1){
        perror("recv failed");
    }

    if(readSize == 0)
    {
        puts("Client disconnected");
        fflush(stdout);
    }

    // Checking if the requested file exists
    FILE* filePointer;
    char response[DEFAULT_BUFLEN];
    clientRequest[readSize-1] = '\0'; 

    HttpRequest parsedRequest;
    if(ParseRequest(clientRequest, &parsedRequest) != 0){
        strcpy(response, "Invalid request format!\n");
        if( send(*clientSock , response , strlen(response), 0) < 0)
        {
            puts("Send failed");
        }
        close(*clientSock);
        return NULL;
    }
    
    if(strcmp(parsedRequest.method, "GET") == 0){
        if(strcmp(parsedRequest.protocol, "HTTP/1.1") == 0){
            if(FileExists(parsedRequest.path)){
                strcpy(response, "HTTP/1.1 200 OK\n\nContent-Type: text/html; charset=UTF-8\n\nConnection:close\n\n");
                if(filePointer = fopen(parsedRequest.path, "r")){
                    char tmp[64];
                    while(fgets(tmp, 64, filePointer) != NULL){
                        strcat(response, tmp);
                    }
                }else{
                    puts("Error opening file!");
                }

                fclose(filePointer);

            }else{ 
                strcpy(response, "HTTP/1.1 404 Page not found\n\nConnection: close\n\n");
            }
        }else{
            strcpy(response, "Unknown protocol!\n");
            if( send(*clientSock , response , strlen(response), 0) < 0)
            {
                puts("Send failed");
            }
            close(*clientSock);
            return NULL;
        }
    }else{
        strcpy(response, "Unknown method!\n");
        if( send(*clientSock , response , strlen(response), 0) < 0)
        {
            puts("Send failed");
        }
        close(*clientSock);
        return NULL;
    }

    // Sending the response to the client
    if( send(*clientSock , response , strlen(response), 0) < 0)
    {
        puts("Send failed");
    }
    
    close(*clientSock);
    return NULL;
}

int main(int argc , char *argv[]){
    int socketDesc, c;
    struct sockaddr_in server , client;
    
    pthread_t hReciever;
   
    // Creating the socket
    socketDesc = socket(AF_INET , SOCK_STREAM , 0);
    if (socketDesc == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");
     
    // Preparing the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(DEFAULT_PORT);

    // Binding
    if( bind(socketDesc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        // Printing the error message
        perror("bind failed. Error");
        return 1;
    }
    puts("bind done");

    // Listening
    listen(socketDesc , 3);

    // Accepting and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
    
     while(1){
    	int* clientSock = (int*)malloc(sizeof(int*));
    	*clientSock = accept(socketDesc, (struct sockaddr *)&client, (socklen_t*)&c);
    	sockData clientTmpSock;
    	clientTmpSock.socket = clientSock;
    	if(*clientSock < 0){
    		perror("accept failed");
    		return 1;
    	}
    	puts("Connection accepted");
    	pthread_create(&hReciever, NULL, Worker, (void*)&clientTmpSock);
    	
    }
 
    return 0;
}
