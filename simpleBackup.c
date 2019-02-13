#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include "simpleBackupConstants.h"


/**
 * WORKING
 * @fileToSend string with the path of file. The file source and fileToSend are the same.
 */
int sendFile(int connectionSocket, char fileToSend[], struct fileTransfering* message)
{    
    int source = open(fileToSend, O_RDONLY);
    ssize_t readedFromFile = 0, writenOnFile = 0;
    struct stat fileInfo;

    if(fileToSend == NULL) return -1;

    //STARTING FILE
    message->code = BACKUP_FILE_NAME;
    strcpy(message->buffer, fileToSend);
    message->messageSize = strlen(fileToSend);
    send(connectionSocket, message, sizeof(struct fileTransfering), 0);

    do
    {
        readedFromFile = read(source, message->buffer, MESSAGE_BUFFER_SIZE);
        message->messageSize = (unsigned int) readedFromFile;
        message->code = BACKUP_FILE_STREAM;
        send(connectionSocket, message, sizeof(struct fileTransfering), 0);
    }
    while(readedFromFile >= MESSAGE_BUFFER_SIZE);
    
    //FINISHING FILE
    memset(message->buffer, 0, MESSAGE_BUFFER_SIZE);

    fstat(source, &fileInfo);
    sprintf(message->buffer, "%u", fileInfo.st_mode);
    message->messageSize = strlen(message->buffer);
    message->code = BACKUP_FILE_END;

    send(connectionSocket, message, sizeof(struct fileTransfering), 0);
    close(source);
    return 0;
}

/**
 * WORKING
 * @fileToSend string with the path of file source.
 */
int sendFolder(int connectionSocket, char streamToSend[], struct fileTransfering* message)
{
    DIR *source = opendir(streamToSend);
    struct dirent *sourceFolderEntry = NULL;
    char destinationName[MESSAGE_BUFFER_SIZE] = "\0";

    if(!source)
    {
        printf("no folder\n");
        return -1;
    }

    //STARTING NEW FOLDER
    message->code = BACKUP_FOLDER;
    message->messageSize = 0;
    memset(message->buffer, 0, MESSAGE_BUFFER_SIZE);
    send(connectionSocket, message, sizeof(struct fileTransfering), 0);

    //SENDING FOLDER`S NAME
    message->code = BACKUP_FOLDER_NAME;
    strcpy(message->buffer, streamToSend);
    message->messageSize = strlen(message->buffer);

    send(connectionSocket, message, sizeof(struct fileTransfering), 0);

    do
    {
        sourceFolderEntry = readdir(source);
        if(sourceFolderEntry != NULL)
        {
            if(sourceFolderEntry->d_type != DT_DIR)
            {
                message->code = BACKUP_FILE;
                message->messageSize = 0;
                memset(message->buffer, 0, MESSAGE_BUFFER_SIZE);
                send(connectionSocket, message, sizeof(struct fileTransfering), 0);
                strcpy(destinationName,streamToSend);
                strcat(destinationName, "/");
                strcat(destinationName, sourceFolderEntry->d_name);
                sendFile(connectionSocket, destinationName, message);
            }
            else
            {
                if(strcmp(".", sourceFolderEntry->d_name) != 0 && strcmp("..", sourceFolderEntry->d_name) != 0)
                {
                    strcpy(destinationName,streamToSend);
                    strcat(destinationName, "/");
                    strcat(destinationName, sourceFolderEntry->d_name);
                    sendFolder(connectionSocket, destinationName, message);
                }
            }
        }
    }
    while(sourceFolderEntry != NULL);

    closedir(source);
    return 0; 
}

void startBackup(int socket, char folderToBackup[], struct fileTransfering* message)
{
    message->code = START_BACKUP;
    printf("STARTING TRANSMITING BACKUP\n");

    memset(message->buffer, 0, MESSAGE_BUFFER_SIZE);
    send(socket, message, sizeof(struct fileTransfering), 0);

    sendFolder(socket, folderToBackup, message);
    
    message->code = FINISH_BACKUP;
    printf("FINISHING TRANSMITING BACKUP\n");

    memset(message->buffer, 0, MESSAGE_BUFFER_SIZE);
    send(socket, message, sizeof(struct fileTransfering), 0);

    return;
}

int main(int argc, char* argv[])
{
    //tcp socket
    
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0}, pin[10] = {0}, backupFolder[MESSAGE_BUFFER_SIZE];
    char *hello = "Hello from server";
    struct fileTransfering message;
    int process = 0;
    char code = 'n';

    if(argc < 2){
        perror("NO BACKUP FOLDER\n");
        return -1;  
    }

    strcpy(backupFolder, argv[1]);
    
    //testing the message
    message.code = ESTABLISH_CONNECTION;
    message.messageSize = 0;
    memset(message.buffer, 0, MESSAGE_BUFFER_SIZE);
    
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    
    // Allowing reuse of IP address
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR ,
                   &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(10015);
    
    // Forcefully attaching socket to the port 10015
    if (bind(server_fd, (struct sockaddr *)&address,
             sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    while(1)
    {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                                                (socklen_t*)&addrlen)) < 0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        process = fork();
        if(process == 0) break;
        sleep(0);
    }
    
    send(new_socket , (void*) &message, sizeof(struct fileTransfering), 0);
    printf("ESTABLISHING CONNECTION\n");
    
    recv(new_socket, (void*) &message, sizeof(struct fileTransfering), 0);
    
    //Saving pin to show connections history
    strncpy(pin, message.buffer, 10);

    printf("CONNECTED WITH PIN %s\n", message.buffer);
    printf("IS THIS A TRUSTED CONNECTION? Y or n?\n");
    fflush(stdin);
    scanf("%c", &code);

    if(code == 'Y')
    {
        message.code = PIN_ACCEPTED;
        memset(message.buffer, 0, MESSAGE_BUFFER_SIZE);
        
        send(new_socket, (void*)&message, sizeof(struct fileTransfering), 0);

        startBackup(new_socket, backupFolder, &message);
    }
    else
    {
        message.code = PIN_ERROR;
        printf("CONNECTION NOT ACCEPTED\n");    
        send(new_socket, (void*)&message, sizeof(struct fileTransfering), 0);
    }
    
    printf("CLOSING CONNECTION PIN : %s\n", pin);
    
    //Finishinhg connection
    message.code = FINISH_CONNECTION;
    send(new_socket, (void*)&message, sizeof(struct fileTransfering), 0);
    close(new_socket);

    exit(0);
}
