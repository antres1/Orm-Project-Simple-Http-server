/* 
    ********************************************************************
    Odsek:          Elektrotehnika i racunarstvo
    Departman:      Racunarstvo i automatika
    Katedra:        Racunarska tehnika i racunarske komunikacije (RT-RK)
    Predmet:        Osnovi Racunarskih Mreza 1
    Godina studija: Treca (III)
    Skolska godina: 2021/22
    Semestar:       Zimski (V)
    
    Ime fajla:      client.c
    Opis:           TCP/IP klijent
    
    Platforma:      Raspberry Pi 2 - Model B
    OS:             Raspbian
    ********************************************************************
*/

#include<stdio.h>      //printf
#include<string.h>     //strlen
#include<sys/socket.h> //socket
#include<arpa/inet.h>  //inet_addr
#include <fcntl.h>     //for open
#include <unistd.h>    //for close

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT   27015

int main(int argc , char *argv[])
{
    int sock;
    struct sockaddr_in server;
    char request[DEFAULT_BUFLEN];
    
    printf("Request format: get <path> HTTP/1.1\nEnter HTTP request: ");
    fgets(request, DEFAULT_BUFLEN, stdin);
    puts(request);

    //Create socket
    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");

    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons(DEFAULT_PORT);

    //Connect to remote server
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("connect failed. Error");
        return 1;
    }

    puts("Connected\n");

    //Send some data
    if( send(sock , request , strlen(request), 0) < 0)
    {
        puts("Send failed");
        return 1;
    }
    
    //prijem odgovora iz servera
    char server_response[DEFAULT_BUFLEN];
    int read_size;
    read_size = recv(sock , server_response , DEFAULT_BUFLEN , 0);
    if(read_size == -1){
        perror("recv failed");
    }
    
    server_response[read_size] = '\0';
    puts(server_response);
    
    close(sock);

    return 0;
}

