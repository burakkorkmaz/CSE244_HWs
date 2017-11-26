#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <signal.h>
#include <wait.h>
#include <arpa/inet.h>
#include <sys/shm.h>
#include <math.h>
#include <sys/time.h>
#include "sharedArea.h"

#define ARGUMENTS 3
#define HOSTNAME_MAX 255
#define QUEUE_SIZE 10
#define MAX_REQUEST 250

#define LOCALHOST "127.0.0.1"


#define MAX_CONTENT 4096;
/* Global Variables */
int exitFlag;
int socketFD;
int connectionFD = 0;
pthread_mutex_t mutex;
pid_t pid;
int shmID;


int establishSocket(unsigned short portNum);

int getConnection(int sock);

void fireman(void);

void signalHandler(int sigNum);


void matrixGenerator(double matrix[][MAX_SIZE_MATRIX], int mCol, int pRow);


void printMatrix(double M[][MAX_SIZE_MATRIX], int mCol, int pRow);

void copyMatrix(double sourceMatrix[][MAX_SIZE_MATRIX], double destMatrix[][MAX_SIZE_MATRIX], int mCol, int pRow);

void multiplyMatrices(double first[][MAX_SIZE_MATRIX], int mCol1, int pRow1,
                      double second[][MAX_SIZE_MATRIX],int mCol2, int pRow2,
                      double result[][MAX_SIZE_MATRIX]);

void tranposeMatrix(double original[][MAX_SIZE_MATRIX], double transpose[][MAX_SIZE_MATRIX], int mCol, int pRow);

void p1Generate(threadArg_t arg);

void p2Solve();

int p3Verify(int connectFD);

void *threadSVDecomposition(void *arg);

void *threadQRFactoring(void *arg);

void *threadPInverse(void *arg);

void substractMatrices(double first[][MAX_SIZE_MATRIX], double second[][MAX_SIZE_MATRIX], int mCol, int pRow,
                       double result[][MAX_SIZE_MATRIX]) ;

void writeMatrixToFile(double M[][MAX_SIZE_MATRIX], int mCol, int pRow,FILE* log) ;

/*
 * A(mxp) . X(px1) = b(mx1)
 */


void *performRequest(void *arg) {

    int mColA, pRowA;
    int connectionFD = *(int *) (arg);
    threadArg_t threadArg;


    read(connectionFD, &threadArg, sizeof(threadArg_t));
    pid = threadArg.pid;
    mColA = threadArg.mCol;
    pRowA = threadArg.pRow;
    fprintf(stderr, "client pid: %d\n", pid);
    fprintf(stderr, "Matrix A %dx%d\n", pRowA, mColA);

    //create shared Memory with unique key
    shmID = shmget((key_t) (IPC_PRIVATE + pthread_self() + pid), sizeof(threadArg_t), IPC_CREAT | 0600);


    pid_t childP1;
    pid_t childP2;
    pid_t childP3;

    childP1 = fork();
    if (childP1 == 0) {
        p1Generate(threadArg);
        exit(TRUE);
    } else { /* Current Process */

        childP2 = fork();
        if (childP2 == 0) {
            p2Solve();
            exit(TRUE);
        } else { /* Current Process */

            childP3 = fork();
            if (childP3 == 0) {
                p3Verify(connectionFD);
                exit(TRUE);
            } else { /* Current Process */

            }

        }

    }

    /*struct timeval start,end;
    int runningTime = 0;
    gettimeofday(&start, NULL);

    gettimeofday(&end, NULL);
    runningTime = (end.tv_sec * AMILLION + end.tv_usec) - (start.tv_sec * AMILLION + start.tv_usec);*/




    while (wait(NULL) > 0);





    FILE * logFD;
    char filename[255];
    memset(filename,0,255);
    sprintf(filename,"log/server/[%d]%lu.txt",pid,pthread_self());
    logFD = fopen((const char *) filename, "a+");

    fprintf(logFD,"Pid: %d\n Thread ID: %lu\n MAtrix A %dx%d\n",pid,pthread_self(), pRowA,mColA);

    writeMatrixToFile(threadArg.matrixA,mColA,pRowA,logFD);

    fprintf(logFD,"Matrix B %dx%d\n",pRowA,VECTOR);
    writeMatrixToFile(threadArg.matrixB,VECTOR,pRowA,logFD);

    fprintf(logFD,"Matrix x1 %dx%d\n",mColA,VECTOR);
    writeMatrixToFile(threadArg.matrix_X1,VECTOR,mColA,logFD);

    fprintf(logFD,"Matrix x2 %dx%d\n",mColA,VECTOR);
    writeMatrixToFile(threadArg.matrix_X2,VECTOR,mColA,logFD);

    fprintf(logFD,"Matrix x3 %dx%d\n",mColA,VECTOR);
    writeMatrixToFile(threadArg.matrix_X3,VECTOR,mColA,logFD);

    fprintf(logFD,"Norm of Error 1: %lf\n",threadArg.error1);
    fprintf(logFD,"Norm of Error 2: %lf\n",threadArg.error2);
    fprintf(logFD,"Norm of Error 3: %lf\n",threadArg.error3);

    fclose(logFD);

    close(connectionFD);
    pthread_exit(NULL);
}

int main(int argc, const char **argv) {
    int portId;

    if (argc != ARGUMENTS) {
        fprintf(stderr, "Usage: server <port number, id> <thread pool size, k>\nFor thread-per-request, "
                "give k is zero (0).\n");
        return ERROR;
    }

    portId = atoi(argv[1]);
    if ((socketFD = establishSocket((unsigned short) portId)) < 0) {
        fprintf(stderr, "Socket Establish Failed.\n");
        close(socketFD);
        exit(TRUE);
    }
    fprintf(stderr, "\nServer started... %d\n", socketFD);

    struct sigaction sa;
    sa.sa_handler = &signalHandler;
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
    pthread_t pthreads[MAX_REQUEST];

    int i = 0;
    int connFDArr[MAX_REQUEST];
    while (!exitFlag) {
        signal(SIGCHLD, (__sighandler_t) fireman);
        if ((connectionFD = getConnection(socketFD)) < 0) {
            if (errno == EINTR) {
                continue;
            }
            fprintf(stderr, "Connection Failed.\n");
//            close(socketFD);
            exit(TRUE);

        } else {
            connFDArr[i] = connectionFD;
//            printf("ConnFD:%d\n", connectionFD);
            pthread_create(&pthreads[i], NULL, performRequest, &connFDArr[i]);


        }
        ++i;
    }
    for (int j = 0; j < i; ++j) {
        pthread_join(pthreads[j], NULL);
        close(connFDArr[j]);
    }

    fprintf(stderr, "\nServer shutting down...\n");
    close(socketFD);
    close(connectionFD);
    return 0;
}

/* From CSE 244 - System Programming, Network Communication Slides */
int establishSocket(unsigned short portNum) {
    char myname[HOSTNAME_MAX];
    int sock;
    struct sockaddr_in sa;
    struct hostent *hp;

    memset(&sa, 0, sizeof(struct sockaddr_in));
    memset(&myname, 0, sizeof(HOSTNAME_MAX));

    gethostname(myname, HOSTNAME_MAX);
    hp = gethostbyname(myname);
    if (hp == NULL) /* we don't exist !? */
    {
        return ERROR;
    }


    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        return ERROR;


    int a = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                   &a, sizeof(a))) {
        perror("set socket");
        exit(TRUE);
    }

    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = INADDR_ANY;
    sa.sin_port = htons(portNum);

    if (bind(sock, (struct sockaddr *) &sa, sizeof(struct sockaddr_in)) < 0) {
        close(sock);
        return ERROR;
    }
    listen(sock, QUEUE_SIZE);
    return (sock);
}

int getConnection(int sock) {
    int connect;
    if ((connect = accept(sock, NULL, NULL)) < 0)
        return ERROR;
    return (connect);
}

void fireman(void) {
    while (waitpid(ERROR, NULL, WNOHANG) > 0);
}

void signalHandler(int sigNum) {

    if (sigNum == SIGINT) {
        fprintf(stderr, "SIGINT received.\n");
        kill(pid, SIGTERM);
        exitFlag = 1;
        close(socketFD);
        close(connectionFD);
    }
}


void matrixGenerator(double matrix[][MAX_SIZE_MATRIX], int mCol, int pRow) {
    int i, j;
    int random;
    srand((unsigned int)(getpid()+pthread_self()));
    for (i = 0; i < pRow; ++i) {
        for (j = 0; j < mCol; ++j) {
            random = rand() % 19 - 9;    /*-9 ile 9 arası*/
            matrix[j][i] = random;
        }
    }

}

void printMatrix(double M[][MAX_SIZE_MATRIX], int mCol, int pRow) {

    int i, j;
    for (i = 0; i < pRow; ++i) {
        fprintf(stderr, "| ");
        for (j = 0; j < mCol; ++j) {
            fprintf(stderr, "%7.3f ", M[j][i]);
        }
        fprintf(stderr, "|\n");
    }
    fprintf(stderr, "\n");
}


void writeMatrixToFile(double M[][MAX_SIZE_MATRIX], int mCol, int pRow,FILE* log) {

    int i, j;
    for (i = 0; i < pRow; ++i) {
        fprintf(log, "| ");
        for (j = 0; j < mCol; ++j) {
            fprintf(log, "%7.3f ", M[j][i]);
        }
        fprintf(log, "|\n");
    }
    fprintf(log, "\n");
}

void copyMatrix(double sourceMatrix[][MAX_SIZE_MATRIX], double destMatrix[][MAX_SIZE_MATRIX], int mCol, int pRow){

    for (int i = 0; i < mCol; ++i) {
        for (int j = 0; j < pRow; ++j) {
            destMatrix[i][j] = sourceMatrix[i][j];
        }
    }
}

void p1Generate(threadArg_t arg) {

    matrixGenerator(arg.matrixA, arg.mCol, arg.pRow);
    matrixGenerator(arg.matrixB, arg.mCol, VECTOR);
    fprintf(stderr,"Matrix A\n");
    printMatrix(arg.matrixA, arg.mCol, arg.pRow);
    fprintf(stderr,"Matrix B\n");
    printMatrix(arg.matrixB, arg.mCol, VECTOR);

    threadArg_t *shmData = shmat(shmID, NULL, 0);
    *shmData = arg;


}

void p2Solve() {

    pthread_t threadSVD;
    pthread_t threadQR;
    pthread_t threadPI;

    pthread_mutex_init(&mutex, NULL);

    pthread_create(&threadSVD, NULL, threadSVDecomposition, NULL);
    pthread_create(&threadQR, NULL, threadQRFactoring, NULL);
    pthread_create(&threadPI, NULL, threadPInverse, NULL);


    pthread_join(threadSVD, NULL);
    pthread_join(threadQR, NULL);
    pthread_join(threadPI, NULL);

    pthread_mutex_destroy(&mutex);
}

void *threadSVDecomposition(void *arg) {

    pthread_mutex_lock(&mutex);
    fprintf(stderr,"SVD here!\n");
    threadArg_t *shmData = shmat(shmID, NULL, 0);
    double MatrixX[VECTOR][MAX_SIZE_MATRIX];
    int pRow = shmData->pRow;
    matrixGenerator(MatrixX,VECTOR,pRow);
    copyMatrix(MatrixX, shmData->matrix_X1, shmData->pRow, VECTOR);

    pthread_mutex_unlock(&mutex);

}

void *threadQRFactoring(void *arg) {

    pthread_mutex_lock(&mutex);
    fprintf(stderr,"QR Fact here!\n");
    threadArg_t *shmData = shmat(shmID, NULL, 0);
    double MatrixX[VECTOR][MAX_SIZE_MATRIX];
    int pRow = shmData->pRow;
    matrixGenerator(MatrixX,VECTOR,pRow);
    copyMatrix(MatrixX, shmData->matrix_X2, shmData->pRow, VECTOR);

    pthread_mutex_unlock(&mutex);

}

void *threadPInverse(void *arg) {

    pthread_mutex_lock(&mutex);
    fprintf(stderr,"Pseudo Inverse here!\n");

    threadArg_t *shmData = shmat(shmID, NULL, 0);
    double MatrixX[VECTOR][MAX_SIZE_MATRIX];
    int pRow = shmData->pRow;
    matrixGenerator(MatrixX,VECTOR,pRow);
    copyMatrix(MatrixX, shmData->matrix_X3, shmData->pRow, VECTOR);

    pthread_mutex_unlock(&mutex);

}

int p3Verify(int connectFD) {


    double errorNorm1;
    double errorNorm2;
    double errorNorm3;
    double ErrorMatrix[MAX_SIZE_MATRIX][MAX_SIZE_MATRIX];
    double ErrorTransMatrix[MAX_SIZE_MATRIX][MAX_SIZE_MATRIX];
    double ErrorNormMatrix[MAX_SIZE_MATRIX][MAX_SIZE_MATRIX];
    double AXdMatrix[MAX_SIZE_MATRIX][MAX_SIZE_MATRIX];
    threadArg_t *shmData = shmat(shmID, NULL, 0);
    int pRow = shmData->pRow;
    int mCol = shmData->mCol;

    /* Processing   Error = A.Xd - b */
    multiplyMatrices(shmData->matrixA,mCol,pRow,shmData->matrix_X1,VECTOR,mCol,AXdMatrix);
    substractMatrices(AXdMatrix,shmData->matrixB,VECTOR,pRow,ErrorMatrix);
    /* Processing   E = Error.ErrorT */
    tranposeMatrix(ErrorMatrix,ErrorTransMatrix,VECTOR,pRow);
    multiplyMatrices(ErrorMatrix,VECTOR,pRow,ErrorTransMatrix,pRow,VECTOR,ErrorNormMatrix);
    /* Find Error Norm */
    errorNorm1 = ErrorNormMatrix[0][0];
    errorNorm1 = sqrt(errorNorm1);
    fprintf(stderr,"Error1 : %.2f\n",errorNorm1);

    /* Processing   Error = A.Xd - b */
    multiplyMatrices(shmData->matrixA,mCol,pRow,shmData->matrix_X2,VECTOR,mCol,AXdMatrix);
    substractMatrices(AXdMatrix,shmData->matrixB,VECTOR,pRow,ErrorMatrix);
    /* Processing   E = Error.ErrorT */
    tranposeMatrix(ErrorMatrix,ErrorTransMatrix,VECTOR,pRow);
    multiplyMatrices(ErrorMatrix,VECTOR,pRow,ErrorTransMatrix,pRow,VECTOR,ErrorNormMatrix);
    /* Find Error Norm */
    errorNorm2 = ErrorNormMatrix[0][0];
    errorNorm2 = sqrt(errorNorm2);
    fprintf(stderr,"Error2 : %.2f\n",errorNorm2);

    /* Processing   Error = A.Xd - b */
    multiplyMatrices(shmData->matrixA,mCol,pRow,shmData->matrix_X3,VECTOR,mCol,AXdMatrix);
    substractMatrices(AXdMatrix,shmData->matrixB,VECTOR,pRow,ErrorMatrix);
    /* Processing   E = Error.ErrorT */
    tranposeMatrix(ErrorMatrix,ErrorTransMatrix,VECTOR,pRow);
    multiplyMatrices(ErrorMatrix,VECTOR,pRow,ErrorTransMatrix,pRow,VECTOR,ErrorNormMatrix);
    /* Find Error Norm */
    errorNorm3 = ErrorNormMatrix[0][0];
    errorNorm3 = sqrt(errorNorm3);
    fprintf(stderr,"Error3 : %.2f\n",errorNorm3);

    shmData->error1 = errorNorm1;
    shmData->error2 = errorNorm2;
    shmData->error3 = errorNorm3;

    write(connectFD, shmData, sizeof(threadArg_t));

}

void multiplyMatrices(double first[][MAX_SIZE_MATRIX], int mCol1, int pRow1,
                      double second[][MAX_SIZE_MATRIX],int mCol2, int pRow2,
                      double result[][MAX_SIZE_MATRIX]) {

    if (mCol1 != pRow2) {
        fprintf(stderr, "Column of First Matrix and Row of Second Matrix must be equal!\n");
        exit(TRUE);
    }

    for (int i = 0; i < mCol1; ++i) {
        for (int j = 0; j < pRow2; ++j) {
            result[i][j] = 0;
        }
    }

    for (int i = 0; i < pRow1; ++i) {
        for (int j = 0; j < mCol2; ++j) {
            for (int k = 0; k < mCol1; ++k) {
                result[j][i] += first[k][i] * second[j][k];
            }
        }
    }

}

void substractMatrices(double first[][MAX_SIZE_MATRIX], double second[][MAX_SIZE_MATRIX], int mCol, int pRow,
                       double result[][MAX_SIZE_MATRIX]) {


    for (int i = 0; i < mCol; ++i) {
        for (int j = 0; j < pRow; ++j) {
            result[i][j] = 0;
        }
    }

    for (int i = 0; i < mCol; ++i) {
        for (int j = 0; j < pRow; ++j) {
            result[j][i] = first[j][i] - second[j][i];
        }
    }

}


void tranposeMatrix(double original[][MAX_SIZE_MATRIX], double transpose[][MAX_SIZE_MATRIX], int mCol, int pRow){

    for (int i = 0; i < mCol; ++i) {
        for (int j = 0; j < pRow; ++j) {
            transpose[i][j] = original[j][i];
        }
    }
}


