#define BUFFER_SIZE 4096 
#define MESSAGE_SIZE 50

struct fileTransfering{
    int code;
    unsigned int messageSize;
    char buffer[BUFFER_SIZE];
};