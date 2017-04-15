//
// Created by Burak Kağan Korkmaz on 13.04.2017.
//

/*
 * LIBRARY INCLUDES
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>

/*
 * NAME CONSTANTS
 */
#define ARGUMENTS 4
#define MIN_MATRIX_SIZE 1
#define MAX_MATRIX_SIZE 10
#define ERROR -1
#define PERMISSIONS (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define RESPONSE_FIFO "response"
/* Global Variables*/
int exitFlag = 0;


/* FUNCTION PROTOTYPES */
void signalHandler(int intSignalNumber);

int main(int argc, char **argv) {

    int intMatrixSize;
    char *chPtrMainFifo;
    int intMainFileDescriptor;
    pid_t pid = getpid();
    int intStatus = 0;

    if (argc != ARGUMENTS) {
        fprintf(stderr, "Usage: %s <ticks in milliseconds> <matrix size n> <main fifoname>\n", argv[0]);
        return ERROR;
    }

    intMatrixSize = atoi(argv[2]);
    if (intMatrixSize < MIN_MATRIX_SIZE || intMatrixSize > MAX_MATRIX_SIZE) {
        fprintf(stderr, "Error: Invalid Matrix Size!\n");
        fprintf(stderr, "Usage: %s <ticks in milliseconds> <matrix size n> <main fifoname>\n", argv[0]);

    }


    struct sigaction structSigAction;

    structSigAction.sa_handler = &signalHandler;

    if (sigaction(SIGTERM, &structSigAction, NULL) < 0) {
        perror("sig error");
        return 1;
    }

    if (sigaction(SIGINT, &structSigAction, NULL) < 0) {
        perror("sigaction");
        return 1;
    }

    chPtrMainFifo = argv[3];
    if (mkfifo(chPtrMainFifo, PERMISSIONS) == ERROR) {
        fprintf(stderr, "Creating FIFO has failed: %s\n",chPtrMainFifo);
        return 2;
    }
    if (mkfifo(RESPONSE_FIFO, PERMISSIONS) == ERROR) {
        fprintf(stderr, "Creating FIFO has failed: %s\n",RESPONSE_FIFO);
        return 2;
    }

    if ((intMainFileDescriptor = open(chPtrMainFifo, O_WRONLY)) == -1) {
        fprintf(stderr, "FIFO could not open: %s\n",chPtrMainFifo);
        return 2;
    }

    intStatus = (int) write(intMainFileDescriptor,&pid,sizeof(pid_t));
    if(intStatus == ERROR){
        fprintf(stderr, "Writing to FIFO has failed: %s\n",chPtrMainFifo);
        return 2;
    }

    while (!exitFlag) {
       /* printf("processing.\n");
        usleep(100000);
        printf("processing..\n");
        usleep(100000);
        printf("processing...\n");
        usleep(100000);*/
    }

    /* mainfifo unlinking */
    unlink(chPtrMainFifo);

    return 0;
}

void signalHandler(int intSignalNumber) {

    printf("SIG NUMBER: %d\n", intSignalNumber);
    if (intSignalNumber == SIGINT) {
        printf("Signal INTERRUPT has been received!\n");
        exitFlag = 1;
    } else {
        printf("Another signal detected.\n");
        exitFlag = 0;
    }

}

