#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <semaphore.h>
#include <sys/shm.h>
#include "sharedArea.h"

#define ARGUMENTS 5



pthread_mutex_t mutex;
sem_t semID;
threadArg_t threadArgs[MAX_CLIENTS];



int connectToServer(int portNumber);

void printMatrix(double M[][MAX_SIZE_MATRIX], int mCol, int pRow);

void signalHandler(int sigNum) ;

void writeMatrixToFile(double M[][MAX_SIZE_MATRIX], int mCol, int pRow,FILE* log) ;

void* threadFunction(void* args){

    threadArg_t *threadArg = (threadArg_t *) args;
    int socketFD = threadArg->sockFD;

    int value = sem_getvalue(&semID,&value);
    while(value != 0)
        value = sem_getvalue(&semID,&value);



    fprintf(stderr,"PID: %d \n", getpid());
//    fprintf(stderr,"socket: %hu\n",socketFD);
    fprintf(stderr,"Col A: %d\n",threadArg->mCol);
    fprintf(stderr,"Row A: %d\n",threadArg->pRow);
    threadArg->pid = getpid();
    int mCol = threadArg->mCol;
    int pRow = threadArg->pRow;

    write(socketFD,threadArg,sizeof(threadArg_t));

    read(socketFD,threadArg,sizeof(threadArg_t));


    close(socketFD);
    fprintf(stderr,"Matrix A\n");
    printMatrix(threadArg->matrixA,mCol,pRow);
    fprintf(stderr,"Matrix B\n");
    printMatrix(threadArg->matrixB,pRow,VECTOR);


    FILE * logFD;
    char filename[255];
    memset(filename,0,255);
    sprintf(filename,"log/clients/[%d]%lu.txt",getpid(),pthread_self());
    logFD = fopen((const char *) filename, "a+");

    fprintf(logFD,"Pid: %d\n Thread ID: %lu\n MAtrix A %dx%d\n",getpid(),pthread_self(), pRow,mCol);

    writeMatrixToFile(threadArg->matrixA,mCol,pRow,logFD);

    fprintf(logFD,"Matrix B %dx%d\n",pRow,VECTOR);
    writeMatrixToFile(threadArg->matrixB,VECTOR,pRow,logFD);

    fprintf(logFD,"Matrix x1 %dx%d\n",mCol,VECTOR);
    writeMatrixToFile(threadArg->matrix_X1,VECTOR,mCol,logFD);

    fprintf(logFD,"Matrix x2 %dx%d\n",mCol,VECTOR);
    writeMatrixToFile(threadArg->matrix_X2,VECTOR,mCol,logFD);

    fprintf(logFD,"Matrix x3 %dx%d\n",mCol,VECTOR);
    writeMatrixToFile(threadArg->matrix_X3,VECTOR,mCol,logFD);

    fprintf(logFD,"Norm of Error 1: %f\n",threadArg->error1);
    fprintf(logFD,"Norm of Error 2: %f\n",threadArg->error2);
    fprintf(logFD,"Norm of Error 3: %f\n",threadArg->error3);

    fclose(logFD);

    pthread_exit(NULL);
}

int main(int argc, const char** argv) {
    int socketFD;
    int mCol,pRow;
    int qClients;
    int portNum;
    pthread_t threads[MAX_CLIENTS];

    if(argc != ARGUMENTS){
        fprintf(stderr,"Usage: clients <num of col A, m> <# of row A, p> <# of clients, q> <port # of server, id>\n");
        return ERROR;
    }
    struct sigaction sa;
    sa.sa_handler = &signalHandler;
    sigaction(SIGINT,&sa,NULL);
    sigaction(SIGTERM,&sa,NULL);

    mCol = atoi(argv[1]);
    pRow = atoi(argv[2]);
    qClients = atoi(argv[3]);
    portNum = atoi(argv[4]);



    if(mCol <= 0 || pRow <= 0|| mCol > MAX_SIZE_MATRIX || pRow > MAX_SIZE_MATRIX){
        fprintf(stderr,"Invalid Matrix Sizes!(Valid: 1 - 20)\n");
        return ERROR;
    } else if(qClients < 0){
        fprintf(stderr,"Invalid Number of Clients!\n");
        return ERROR;
    }

    sem_init(&semID,0,1);
    sem_wait(&semID);
    for (int i = 0; i < qClients; ++i) {
        threadArgs[i].mCol = mCol;
        threadArgs[i].pRow = pRow;
        socketFD = connectToServer(portNum);
//        printf("SocketFD:%d\n",socketFD);
        if(socketFD == ERROR){
            fprintf(stderr,"Server Connection Failed!\n");
            exit(TRUE);
        }
        threadArgs[i].sockFD = socketFD;
        pthread_create(&threads[i], NULL,  threadFunction, &threadArgs[i]);
    }
    sem_post(&semID);

    for (int j = 0; j < qClients; ++j) {
        pthread_join(threads[j],NULL);
    }

    close(socketFD);
    return 0;
}

int connectToServer(int portNumber){

    struct sockaddr_in serverAddr;
    int sockFD;

    if((sockFD = socket(AF_INET, SOCK_STREAM, 0)) == ERROR)
        return ERROR;

    memset(&serverAddr,0,sizeof(serverAddr));

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(LOCALHOST);
    serverAddr.sin_port = htons((in_port_t)portNumber);

    if (connect(sockFD , (struct sockaddr *)&serverAddr , sizeof(serverAddr)) == ERROR) {
        return ERROR;
    }

//    fprintf(stderr,"soc: %d\n",sockFD);
    return sockFD;
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

void signalHandler(int sigNum) {

    if(sigNum == SIGINT){
        fprintf(stderr,"SIGINT received.\n");
//        close(socketFD);
        exit(TRUE);
    }
    if(sigNum == SIGTERM){
        fprintf(stderr,"SIGTERM received.\n");
//        close(socketFD);
        exit(TRUE);
    }
}