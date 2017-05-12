/*
 * Created by Burak Kağan Korkmaz on 13.04.2017.
 */

/*
 * LIBRARY INCLUDES
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>


/*
 * NAME CONSTANTS
 */
#define ARGUMENTS 4
#define MIN_MATRIX_SIZE 1
#define MAX_MATRIX_SIZE 10
#define ERROR -1
#define PERMISSIONS (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define MAX_CLIENT 255

/* Global Variables*/
int globalIntExitFlag = 0;
int globalIntRequestSignal = 1;
int clientPidsArr[MAX_CLIENT];

/* FUNCTION PROTOTYPES */
static void signalHandler(int intSignalNumber);

double **allocateMatrix(int intSizeOfMatrix);

double **matrixGenerator(int intSizeOfMatrix);

void printMatrix(double **M, int n);

void swapRows(double **matrix, int nRow, int size);

double determinantOfMatrix(double **A, int n);




int main(int argc, char **argv) {
    int i,j,k;
    int intMatrixSize;
    char *chPtrMainFifo;
    int intMainFileDescriptor;
    pid_t pidServer = getpid();
    pid_t pidClient = 0;
    pid_t pidNewServer;
    pid_t pidClientOld = 0;
    int intStatus = 0;
    int intNewFileDescriptor;
    int intSizeOfFifoName = 0;
    char* charPtrFifoName;
    double **dbPtrPtrA;
    clientPidsArr[0] = 0;   /* Keep number of clients */
    struct sigaction structSigAction;

    structSigAction.sa_handler = &signalHandler;

    if (argc != ARGUMENTS) {
        fprintf(stderr, "Usage: %s <ticks in milliseconds> <matrix size n> <main fifoname>\n", argv[0]);
        return ERROR;
    }


    intMatrixSize = atoi(argv[2]);
    if (intMatrixSize < MIN_MATRIX_SIZE || intMatrixSize > MAX_MATRIX_SIZE) {
        fprintf(stderr, "Error: Invalid Matrix Size!\n");
        fprintf(stderr, "Usage: %s <ticks in milliseconds> <matrix size n> <main fifoname>\n", argv[0]);
        return ERROR;
    }

    fprintf(stderr, "TimeServer started at PID %d\n", getpid());
    /* 2n Matrix A generates */
    intMatrixSize *= 2;


	if (sigaction(SIGINT, &structSigAction, NULL) < 0) {
        perror("Sigaction Failed.");
        return 1;
    }
    if (sigaction(SIGTERM, &structSigAction, NULL) < 0) {
        perror("Sigaction Failed.");
        return 1;
    }
    if (sigaction(SIGUSR1, &structSigAction, NULL) < 0) {
        perror("Sigaction Failed.");
        return 1;
    }
    
    chPtrMainFifo = argv[3];
	unlink(chPtrMainFifo);
    if (mkfifo(chPtrMainFifo, PERMISSIONS) == ERROR) {
        fprintf(stderr, "Creating FIFO has failed: %s\n", chPtrMainFifo);
        return 2;
    }
    else if ((intMainFileDescriptor = open(chPtrMainFifo, O_RDONLY)) == -1) {
        fprintf(stderr, "FIFO could not open: %s\n", chPtrMainFifo);
        return 2;
    }

	


    while (!globalIntExitFlag) {

        pidClientOld = pidClient;
        intStatus = (int) read(intMainFileDescriptor, &pidClient, sizeof(pid_t));
        if(globalIntRequestSignal == 1 ){
            while(determinantOfMatrix(dbPtrPtrA = matrixGenerator(intMatrixSize),intMatrixSize) == 0.0);
            printMatrix(dbPtrPtrA, intMatrixSize);


            k = ++clientPidsArr[0];
            clientPidsArr[k] = pidClient;


            if(intStatus > 0 && pidClientOld != pidClient) {
                fprintf(stderr, "Client %d was connected to server...\n",pidClient);
                if ((pidNewServer = fork()) == ERROR) {
                    fprintf(stderr, "Fork Failed!");
                }

                /* New Child Server */
                if (pidNewServer == 0) {

                    pidNewServer = getpid();
                    printf("\nnewServer PID: %d\n", pidNewServer);
                    pidNewServer = pidServer;
                    /* Creating Fifo name for each Client */
                    snprintf(NULL, (size_t) intSizeOfFifoName, "fifo_%d", pidClient);
                    charPtrFifoName = (char *) malloc(intSizeOfFifoName * sizeof(char));
                    sprintf(charPtrFifoName, "fifo_%d", pidClient);

                    if (mkfifo(charPtrFifoName, PERMISSIONS) == ERROR) {
                        fprintf(stderr, "Creating new FIFO has failed: %s\n", charPtrFifoName);
                        return 2;
                    }
                    if ((intNewFileDescriptor = open(charPtrFifoName, O_WRONLY)) == -1) {
                        fprintf(stderr, "new FIFO could not open: %s\n", charPtrFifoName);
                        return 2;
                    }
                    write(intNewFileDescriptor, &pidNewServer, sizeof(pid_t));
                    write(intNewFileDescriptor, &intMatrixSize, sizeof(int));
                    for (i = 0; i < intMatrixSize; ++i) {
                        for (j = 0; j < intMatrixSize; ++j) {
                            write(intNewFileDescriptor, &dbPtrPtrA[i][j], sizeof(double));
                        }
                    }

                    free(charPtrFifoName);
                    exit(0);
                } else if (pidNewServer > 0) {

                    printf("Server Pid: %d\nNewServerPid: %d\nClient Pid: %d\n",
                           pidServer, pidNewServer, pidClient);
                    sleep(1);
                    printf("\nprocessing.\n");
                    /*usleep(100000);*/
                    printf("\nprocessing..\n");
//                usleep(100000);
                    printf("\nprocessing...\n");
//                usleep(100000);

                }
            }
            globalIntRequestSignal = 0;
        }
    }
    perror("exit");
    /* mainfifo unlinking */
    unlink(chPtrMainFifo);



    return 0;
}

static void signalHandler(int intSignalNumber) {
    int i;
    printf("\nSIG NUMBER: %d\n", intSignalNumber);
    if (intSignalNumber == SIGINT) {
        fprintf(stderr,"Signal INTERRUPT has been received!\n");
        globalIntExitFlag = 1;
        for (i = 1; i <= clientPidsArr[0]; ++i) {
            kill(clientPidsArr[i],SIGTERM);
        }
    } else if (intSignalNumber == SIGUSR1) {
        fprintf(stderr,"Signal USER1 has been received!\n");
        globalIntRequestSignal = 1;
    } else if(intSignalNumber == SIGTERM){
        fprintf(stderr,"SIGTERM signal received.\n");
        globalIntExitFlag = 1;
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
            int random = rand() % 21 - 10;    /*-10 ile 10 arası*/
            matrix[i][j] = random;
            /*printf("%6.2f ", matrix[i][j]);*/
        }
        /*printf("\n");*/
    }
    return matrix;
}

void printMatrix(double **M, int n) {
    int i,j;
    for (i = 0; i < n; ++i) {
        fprintf(stderr, "| ");
        for (j = 0; j < n; ++j) {
            fprintf(stderr, "%7.3f ", M[i][j]);
        }
        fprintf(stderr, "|\n");
    }
    fprintf(stderr, "\n");
}

void swapRows(double **matrix, int nRow, int size) {
    int i;
    double *temp;
    if (size - 1 == nRow) {
        return;
    }

    for (i = nRow; i < size - 1; ++i) {
        {
            if (matrix[nRow + 1][nRow + 1] != 0.0) {
                temp = matrix[nRow];
                matrix[nRow] = matrix[nRow + 1];
                matrix[nRow + 1] = temp;
                return;
            }
        }
    }
}

double determinantOfMatrix(double **A, int n) {
    int i, j, k;
    double ratio;
    double det = 1;
    double **matrix = allocateMatrix(n);
    /* copying the matrix */
    for (i = 0; i < n; ++i) {
        for (j = 0; j < n; ++j) {
            matrix[i][j] = A[i][j];
        }

    }
    /* apply gaussian with pivoting before determinant */
    for (k = 0; k < n; ++k) {
        if (matrix[k][k] == 0.0) {
            swapRows(matrix, k, n);
            det *= -1;
        }
        for (i = k + 1; i < n; ++i) {
            ratio = matrix[i][k] / matrix[k][k];

            for (j = k; j < n; ++j) {
                matrix[i][j] -= ratio * matrix[k][j];
            }
        }
    }
    /* Find determinant of Matrix */
    for (i = 0; i < n; i++)

        det *= matrix[i][i];

    return det;
}
