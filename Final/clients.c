#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define ARGUMENTS 5
#define MAX_CLIENTS 200
#define ERROR -1
#define LOCALHOST "127.0.0.1"

#define MAX_SIZE_MATRIX 100
typedef struct{
    pthread_t threadId;
    int sockFD;
    int mColA;
    int pRowA;
    int pRowB;
}threadArg_t;

threadArg_t threadArgs[MAX_CLIENTS];

int connectToServer(int portNumber);
void printMatrix(double M[][MAX_SIZE_MATRIX], int mCol, int pRow);

void* threadFunction(void* args){

    threadArg_t *threadArg = (threadArg_t *) args;
    threadArg->threadId = pthread_self();
    int mCol = threadArg->mColA;
    int pRow = threadArg->pRowA;
    fprintf(stderr,"ThreadID: %lu\n",threadArg->threadId);
    fprintf(stderr,"port: %hu\n",threadArg->sockFD);
    fprintf(stderr,"Col A: %d\n",threadArg->mColA);
    fprintf(stderr,"Row A: %d\n",threadArg->pRowA);

    int socketFD = connectToServer(threadArg->sockFD);
    pid_t pid = getpid();
    write(socketFD, &pid, sizeof(pid_t));
    write(socketFD,&mCol,sizeof(mCol));
    write(socketFD,&pRow,sizeof(pRow));
    double matrixA[MAX_SIZE_MATRIX][MAX_SIZE_MATRIX];
    read(socketFD,&matrixA,sizeof(matrixA));
    printMatrix(matrixA,mCol,pRow);
    close(socketFD);
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
    mCol = atoi(argv[1]);
    pRow = atoi(argv[2]);
    qClients = atoi(argv[3]);
    portNum = atoi(argv[4]);

    for (int i = 0; i < qClients; ++i) {
        threadArgs[i].mColA = mCol;
        threadArgs[i].pRowA = pRow;
        threadArgs[i].pRowB = pRow;

        threadArgs[i].sockFD = portNum;
        pthread_create(&threads[i], NULL,  threadFunction, &threadArgs[i]);
    }


    for (int j = 0; j < qClients; ++j) {
        pthread_join(threads[j],NULL);
    }


    return 0;
}

int connectToServer(int portNumber){

    struct sockaddr_in serverAddr;
    int sockFD;

    if((sockFD = socket(AF_INET, SOCK_STREAM, 0)) == ERROR)
        return ERROR;
    puts("Socket created");
    memset(&serverAddr,0,sizeof(serverAddr));

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(LOCALHOST);
    serverAddr.sin_port = htons((in_port_t)portNumber);

    if (connect(sockFD , (struct sockaddr *)&serverAddr , sizeof(serverAddr)) == ERROR) {
        fprintf(stderr,"Connection Failed!\n");
        return ERROR;
    }
    puts("Connected\n");


    return sockFD;
}


void printMatrix(double M[][MAX_SIZE_MATRIX], int mCol, int pRow) {
    int i,j;
    for (i = 0; i < pRow; ++i) {
        fprintf(stderr, "| ");
        for (j = 0; j < mCol; ++j) {
            fprintf(stderr, "%7.3f ", M[i][j]);
        }
        fprintf(stderr, "|\n");
    }
    fprintf(stderr, "\n");
}