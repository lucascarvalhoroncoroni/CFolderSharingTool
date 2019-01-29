#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h> 
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "simpleBackupConstants.h"

/**
 * WORKING
 * @origin string with the path of file source.
 * @destiny string with the path of file destiny.
 */
int copyFile(char origin[], char destiny[])
{
    if(origin == NULL && destiny == NULL) return -1;
    
    int source = open(origin, O_RDONLY);
    int destination = open(destiny, O_CREAT | O_RDWR);
    ssize_t readedFromFile = 0, writenOnFile = 0;
    char buffer[BUFFER_SIZE];

    printf("%d\n", destination);

    do
    {
        readedFromFile = read(source, buffer, BUFFER_SIZE);
        printf("%lu\n", readedFromFile);
        writenOnFile = write(destination, buffer, readedFromFile);
        printf("%lu\n", writenOnFile);
    }
    while(readedFromFile >= BUFFER_SIZE);
    
    close(source);
    close(destination);
    return 0;
}


int main(int argc, char* argv[])
{
    struct sockaddr_in address; 
    int sock = 0, valread; 
    struct sockaddr_in serv_addr; 
    char *hello = "Hello from client"; 
    char buffer[1024] = {0};

    struct fileTransfering message;

    //Cleaning the message variable
    memset((void*) &message.buffer, 0, BUFFER_SIZE);
    message.code = 0;
    message.messageSize = 0;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    { 
        printf("\n Socket creation error \n"); 
        return -1; 
    } 
   
    memset(&serv_addr, '0', sizeof(serv_addr)); 
   
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(10015); 
       
    // Convert IPv4 and IPv6 addresses from text to binary form 
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)  
    { 
        printf("\nInvalid address/ Address not supported \n"); 
        return -1; 
    } 
   
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
        printf("\nConnection Failed \n"); 
        return -1; 
    }

    valread = read( sock , (void*)&message, sizeof(struct fileTransfering)); 
    printf("%d\n",message.code);
    printf("%d\n",message.messageSize); 
    printf("%s\n",message.buffer);

    return 0;     
}