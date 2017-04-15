//
// Created by eksor on 13.04.2017.
//

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>


#define ARGUMENTS 2
#define ERROR -1

int main(int argc, char **argv) {

    int intStatus = 0;
    int intMainFileDescripter;
    __pid_t clientPID = getpid();
    __pid_t serverPID;

    if (argc != ARGUMENTS) {
        fprintf(stderr, "Usage: %s <main fifoname>\n", argv[0]);
        return ERROR;
    }

    if ((intMainFileDescripter = open(argv[1], O_RDONLY)) == ERROR) {
        fprintf(stderr, "FIFO failed to open: %s\n", argv[1]);
        return ERROR;
    }

    intStatus = (int) read(intMainFileDescripter, &serverPID, sizeof(pid_t));
    if (intStatus == -1) {
        fprintf(stderr, "FIFO failed to read: %s\n", argv[1]);
        return ERROR;

    }


    printf("Server PID: %d\nClient PID: %d\n", clientPID, serverPID);
    return 0;
}