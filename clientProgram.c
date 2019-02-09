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
 * This function terminates the client and send a message to the server
 */
void goodByeMessage(int socket, char byeMessage[])
{
    struct fileTransfering terminateMessage;
    terminateMessage.code = FINISH_CONNECTION;
    strcpy(terminateMessage.buffer, byeMessage);
    terminateMessage.messageSize = strlen(byeMessage);
    send(socket, &terminateMessage, sizeof(struct fileTransfering), 0);
    close(socket);
    exit(1);
}

void backUpFolder(int connectionSocket, struct fileTransfering* message)
{
    char folderName[MESSAGE_BUFFER_SIZE + 1] = {0};
    
    recv(connectionSocket, message, sizeof(struct fileTransfering), 0);
    
    if(message->code == BACKUP_FOLDER_NAME)
    {
        strncpy(folderName, message->buffer, MESSAGE_BUFFER_SIZE + 1);
        if(mkdir(folderName, 0777) == -1)
        {
            printf("can not create folder\n");
            goodByeMessage(connectionSocket, "COULD NOT MAKE A DIRECTORY");
        }   
    }
    else
    {
        goodByeMessage(connectionSocket, "NO FOLDER NAME");
    }
}

void backUpFile(int connectionSocket, struct fileTransfering* message)
{
    //Avoiding Apollo 13
    char fileName[MESSAGE_BUFFER_SIZE + 1] = {0};
    int fileDesc = 0, writenOnFile = 0;
    recv(connectionSocket, message, sizeof(struct fileTransfering), 0); 
    if(message->code != BACKUP_FILE_NAME) return;
    strncpy(fileName, message->buffer, message->messageSize + 1);
    fileDesc = open(fileName, O_CREAT | O_RDWR);

    //WRITING FILE
    // do
    // {
    //     recv(connectionSocket, message, sizeof(struct fileTransfering), 0);
    //     writenOnFile = write(fileDesc, message->buffer, message->messageSize);
    //     printf("%d\n", writenOnFile);
    //     printf("%d\n", message->messageSize);
    //     fwrite(message->buffer, message->messageSize, sizeof(message->buffer), stdout);
    //     fflush(stdout);
    // }
    // while(message->code == BACKUP_FILE_STREAM);

    recv(connectionSocket, message, sizeof(struct fileTransfering), 0);
    writenOnFile = write(fileDesc, message->buffer, message->messageSize);
    printf("%d\n", message->code);
    printf("%d\n", writenOnFile);
    printf("%d\n", message->messageSize);
    fwrite(message->buffer, sizeof(char), message->messageSize, stdout);
    fflush(stdout);

    recv(connectionSocket, message, sizeof(struct fileTransfering), 0);
    printf("%d\n", message->code);
    
    close(fileDesc);
    if(message->code != BACKUP_FILE_END)
    {
        goodByeMessage(connectionSocket, "NO FILE END REACHED BY THE SERVER");
    }
    
}

/**
 *
 */
void startBackUp(int connectionSocket, struct fileTransfering* message)
{
    printf("STARTING BACKUP\n");

    while(1)
    {
        recv(connectionSocket, message, sizeof(struct fileTransfering), 0);
        printf("%d\n", message->code);
        switch(message->code)
        {
            case BACKUP_FILE:
                printf("BACKUP FILE\n");
                backUpFile(connectionSocket, message);
                break;
            
            case BACKUP_FOLDER:
                printf("BACKUP FOLDER\n");
                backUpFolder(connectionSocket, message);
                break;
            
            default:
                return;
        }
    }
}

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
    char buffer[MESSAGE_BUFFER_SIZE];

    printf("%d\n", destination);

    do
    {
        readedFromFile = read(source, buffer, MESSAGE_BUFFER_SIZE);
        printf("%lu\n", readedFromFile);
        writenOnFile = write(destination, buffer, readedFromFile);
        printf("%lu\n", writenOnFile);
    }
    while(readedFromFile >= MESSAGE_BUFFER_SIZE);
    
    close(source);
    close(destination);
    return 0;
}

void clientStart(int connectionSocket, struct fileTransfering* message)
{
    char pin[10];
    printf("ESTABLISHING CONNECTION\n");
    printf("PLEASE TYPE A VALIDATION PIN : ");
    fflush(stdout);
    fflush(stdin);
    fgets(pin, 10, stdin);

    strncpy(message->buffer, pin, 10);
    fflush(stdout);

    send(connectionSocket, message, sizeof(struct fileTransfering), 0);
    recv(connectionSocket, message, sizeof(struct fileTransfering), 0);

    if(message->code == PIN_ACCEPTED)
    {
        printf("CONNECTION ESTABLISHED\n");        
        return;
    }
    else
    {
        close(connectionSocket);
        printf("CONNECTION COULD NOT BE ESTABLISHED\n");
        exit(0);
    }
}


int main(int argc, char* argv[])
{
    struct sockaddr_in address; 
    int sock = 0, valread; 
    struct sockaddr_in serv_addr; 
    char *hello = "Hello from client"; 
    char buffer[1024] = {0};
    int option = 0, keepAlive = 1;
    struct fileTransfering message;

    //Cleaning the message variable
    memset((void*) &message.buffer, 0, MESSAGE_BUFFER_SIZE);
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

    printf("STARTING\n");

    do
    {
        valread = read( sock , (void*)&message, sizeof(struct fileTransfering)); 
        option = message.code;

        switch(option)
        {
            case ESTABLISH_CONNECTION:
                clientStart(sock, &message);
                break;
            case START_BACKUP:
                startBackUp(sock, &message);
                break;
            case FINISH_CONNECTION:
                //Case where the server finishes the connection.
            default:
                keepAlive = 0;   
        }
    }
    while(keepAlive);

    printf("CLOSING SOCKET\n");
    close(sock);

    exit(0);     
}
