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

#define BUFFER_SIZE 4196 


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

/**
 * WORKING
 * @origin string with the path of file source.
 * @destiny string with the path of file destiny.
 */
int copyFolder(char origin[], char destiny[])
{
    DIR *source = opendir(origin);
    struct dirent *sourceFolderEntry = NULL;
    char sourceName[512] = "\0";
    char destinationName[512] = "\0";

    if(!source)
    {
        printf("no folder\n");
        return -1;
    }

    if(mkdir(destiny, 0777) == -1)
    {
        printf("can not create folder\n");
        return -2;
    }

    do
    {
        sourceFolderEntry = readdir(source);
        if(sourceFolderEntry != NULL)
        {
            if(sourceFolderEntry->d_type != DT_DIR)
            {    
                strcpy(destinationName,destiny);
                strcpy(sourceName,origin);
                strcat(destinationName, "/");
                strcat(destinationName, sourceFolderEntry->d_name);
                strcat(sourceName, "/");
                strcat(sourceName, sourceFolderEntry->d_name);
                printf("%s\n",sourceName);
                printf("%s\n",destinationName);
                copyFile(sourceName, destinationName);
            }
            else
            {
                if(strcmp(".", sourceFolderEntry->d_name) != 0 && strcmp("..", sourceFolderEntry->d_name) != 0)
                {
                    printf("%s\n", sourceFolderEntry->d_name);
                    strcpy(destinationName,destiny);
                    strcpy(sourceName,origin);
                    strcat(destinationName, "/");
                    strcat(destinationName, sourceFolderEntry->d_name);
                    strcat(sourceName, "/");
                    strcat(sourceName, sourceFolderEntry->d_name);
                    printf("%s\n",sourceName);
                    printf("%s\n",destinationName);
                    copyFolder(sourceName, destinationName);
                }
            }
        }
    }
    while(sourceFolderEntry != NULL);

    closedir(source);
}

int main(int argc, char* argv[])
{
    // if(argc < 2){
    //     perror("something wrong");
    //     return -1;  
    // } 
    // printf("%s",argv[1]);
    // printf("\n");
    // printf("%s",argv[2]);
    // copyFolder(argv[1], argv[2]);

    //tcp socket
    int connectionSocket = socket(AF_INET, SOCK_STREAM, 0);



    return 0;
}