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
#include <time.h>


/*
 * NAME CONSTANTS
 */
#define ARGUMENTS 4
#define MIN_MATRIX_SIZE 1
#define MAX_MATRIX_SIZE 10
#define ERROR -1
#define PERMISSIONS (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

#define REQUEST_FIFO "request"

/* Global Variables*/
int globalIntExitFlag = 0;


/* FUNCTION PROTOTYPES */
static void signalHandler(int intSignalNumber);

double **allocateMatrix(int intSizeOfMatrix);

double **matrixGenerator(int intSizeOfMatrix);

void printMatrix(const double **M, int n);

int main(int argc, char **argv) {

    int intMatrixSize;
    char *chPtrMainFifo;
    int intMainFileDescriptor;
    pid_t pidServer = getpid();
    pid_t pidClient;
    pid_t pidNewServer;
    int intStatus = 0;
    int intNewFileDescriptor;
    double **dbPtrPtrA;

    if (argc != ARGUMENTS) {
        fprintf(stderr, "Usage: %s <ticks in milliseconds> <matrix size n> <main fifoname>\n", argv[0]);
        return ERROR;
    }

    fprintf(stderr, "TimeServer started at PID %d\n", getpid());

    intMatrixSize = atoi(argv[2]);
    if (intMatrixSize < MIN_MATRIX_SIZE || intMatrixSize > MAX_MATRIX_SIZE) {
        fprintf(stderr, "Error: Invalid Matrix Size!\n");
        fprintf(stderr, "Usage: %s <ticks in milliseconds> <matrix size n> <main fifoname>\n", argv[0]);

    }

    /* 2n Matrix A generates */
    intMatrixSize *= 2;
    dbPtrPtrA = matrixGenerator(intMatrixSize);
    printMatrix((const double **) dbPtrPtrA, intMatrixSize);

    struct sigaction structSigAction;

    structSigAction.sa_handler = &signalHandler;

    /*if (sigaction(SIGTERM, &structSigAction, NULL) < 0) {
        perror("sig error");
        return 1;
    }*/

    if (sigaction(SIGINT, &structSigAction, NULL) < 0) {
        perror("sigaction");
        return 1;
    }

    chPtrMainFifo = argv[3];
    perror("Create fifo");
    if (mkfifo(chPtrMainFifo, PERMISSIONS) == ERROR) {
        fprintf(stderr, "Creating FIFO has failed: %s\n", chPtrMainFifo);
        return 2;
    }
    if ((intMainFileDescriptor = open(chPtrMainFifo, O_RDONLY)) == -1) {
        fprintf(stderr, "FIFO could not open: %s\n", chPtrMainFifo);
        return 2;
    }
    /*
    intStatus = (int) write(intMainFileDescriptor,&serverPid,sizeof(pid_t));
    if(intStatus == ERROR){
        fprintf(stderr, "Writing to FIFO has failed: %s\n",chPtrMainFifo);
        return 2;
    }*/

    while (!globalIntExitFlag) {


        while (read(intMainFileDescriptor, &pidClient, sizeof(pid_t)) < 1);

        if ((pidNewServer = fork()) == ERROR) {
            fprintf(stderr, "Fork Failed!");
        }
        if (pidNewServer == 0) {
            pidNewServer = getpid();
            if (mkfifo(REQUEST_FIFO, PERMISSIONS) == ERROR) {
                fprintf(stderr, "Creating FIFO has failed: %s\n", chPtrMainFifo);
                return 2;
            }
            if ((intNewFileDescriptor = open(chPtrMainFifo, O_WRONLY)) == -1) {
                fprintf(stderr, "FIFO could not open: %s\n", chPtrMainFifo);
                return 2;
            }
            while(write(intNewFileDescriptor, &pidNewServer, sizeof(pid_t)) < 1);
            while(write(intNewFileDescriptor, &intMatrixSize, sizeof(int)) < 1);

        } else if (pidNewServer > 0) {
            printf("Server Pid: %d\nClient Pid: %d\n", pidServer, pidClient);
            printf("processing.\n");
            usleep(100000);
            printf("processing..\n");
            usleep(100000);
            printf("processing...\n");
            usleep(100000);

        }


    }
    perror("exit");

    /* mainfifo unlinking */
    unlink(chPtrMainFifo);


    return 0;
}

static void signalHandler(int intSignalNumber) {

    printf("SIG NUMBER: %d\n", intSignalNumber);
    if (intSignalNumber == SIGINT) {
        printf("Signal INTERRUPT has been received!\n");
        globalIntExitFlag = 1;
    } else {
        printf("Another signal detected.\n");
        globalIntExitFlag = 0;
    }

}

double **allocateMatrix(int intSizeOfMatrix) {
    double **dbPtrPtrMatrix;
    int i;
    dbPtrPtrMatrix = (double **) malloc(intSizeOfMatrix * sizeof(double *));
    if (!dbPtrPtrMatrix) {
        perror("\nMatrix Allocation Error\n");
        return NULL;
    }

    for (i = 0; i < intSizeOfMatrix; ++i) {
        dbPtrPtrMatrix[i] = (double *) malloc(intSizeOfMatrix * sizeof(double));
        if (!dbPtrPtrMatrix[i]) {
            perror("\nMatrix Allocation Error\n");
            return NULL;
        }
    }
    return dbPtrPtrMatrix;
}

void printMatrix(const double **M, int n) {

    for (int i = 0; i < n; ++i) {
        fprintf(stderr, "| ");
        for (int j = 0; j < n; ++j) {
            fprintf(stderr, "%7.3f ", M[i][j]);
        }
        fprintf(stderr, "|\n");
    }
    fprintf(stderr, "\n");
}

double **matrixGenerator(int n) {
    int i, j, k;
    double **matrix = NULL;

    matrix = allocateMatrix(n);

    for (k = 0; k < n; ++k) {
        matrix[k] = (double *) malloc((n + 1) * sizeof(double));
    }

    srand(time(NULL));
    for (i = 0; i < n; ++i) {
        for (j = 0; j < n; ++j) {
            int random = rand() % 21 - 10;    //-10 ile 10 arası
            matrix[i][j] = random;
            /*printf("%6.2f ", matrix[i][j]);*/
        }
        /*printf("\n");*/
    }
    return matrix;
}