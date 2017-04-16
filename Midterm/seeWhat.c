//
// Created by eksor on 13.04.2017.
//

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>


#define ARGUMENTS 2
#define ERROR -1
#define REQUEST_FIFO "request"

static void signalHandler(int intSignalNumber);

int main(int argc, char **argv) {

    int intStatus = 0;
    int intMainFileDescripter;
    int intNewServerDecripter;
    pid_t pidClient = getpid();
    pid_t pidServer;
    int intMatrixSize = 0;

    if (argc != ARGUMENTS) {
        fprintf(stderr, "Usage: %s <main fifoname>\n", argv[0]);
        return ERROR;
    }

    if ((intMainFileDescripter = open(argv[1], O_WRONLY)) == ERROR) {
        fprintf(stderr, "FIFO failed to open: %s\n", argv[1]);
        return ERROR;
    }
    intStatus = (int) write(intMainFileDescripter, &pidClient, sizeof(pid_t));
    if (intStatus == ERROR) {
        fprintf(stderr, "FIFO failed to read: %s\n", argv[1]);
        return ERROR;

    }
    printf("Client PID: %d\n", pidClient);
    if ((intNewServerDecripter = open(REQUEST_FIFO, O_RDONLY)) == ERROR) {
        fprintf(stderr, "server fifo failed to open: %s\n", argv[1]);
        return ERROR;
    }
    perror("read");
    while (read(intNewServerDecripter, &pidServer, sizeof(pid_t)) < 1);
    while (read(intNewServerDecripter, &intMatrixSize, sizeof(int)) < 1);

    printf("Server PID: %d\n", pidServer);
    printf("Matrix Size: %d\n", intMatrixSize);



    return 0;
}

