#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <signal.h>
#include <wait.h>

#define ARGUMENTS 3
#define ERROR -1


#define HOSTNAME_MAX 255
#define QUEUE_SIZE 10
#define MAX_SIZE_MATRIX 100

#define TRUE 1
#define FALSE 0

/* Global Variables */
int exitFlag;



int establishSocket(unsigned short portNum);

int getConnection(int sock);

void fireman(void) ;

void signalHandler(int sigNum);

double **allocateMatrix(int mCol, int pRow);


void matrixGenerator(double matrix[][MAX_SIZE_MATRIX],int mCol, int pRow);


void printMatrix(double M[][MAX_SIZE_MATRIX], int mCol, int pRow);


void freeMatrix(double **M, int pRow);

int main(int argc, const char **argv) {
    int portId;
    int socketFD;
    int connectionFD;
    struct sigaction sa;
    int mColA, pRowA;

    if (argc != ARGUMENTS) {
        fprintf(stderr, "Usage: server <port number, id> <thread pool size, k>\nFor thread-per-request, "
                "give k is zero (0).\n");
        return ERROR;
    }

    portId = atoi(argv[1]);
    if((socketFD = establishSocket((unsigned short) portId)) < 0){
        fprintf(stderr,"Socket Establish Failed.\n");
        close(socketFD);
        exit(TRUE);
    }
    fprintf(stderr,"\nServer started...\n");
    sa.sa_handler = &signalHandler;
    sigaction(SIGINT,&sa,NULL);
    while (!exitFlag){
        signal(SIGCHLD, (__sighandler_t) fireman);

        if((connectionFD = getConnection(socketFD)) < 0){
            if (errno == EINTR)
                continue;
            fprintf(stderr,"Connection Failed.\n");
//            close(socketFD);
            exit(TRUE);

        } else{
            pid_t  pid;
            read(connectionFD, &pid, sizeof(pid));
            fprintf(stderr, "client pid: %d\n", pid);
            read(connectionFD,&mColA,sizeof(mColA));
            read(connectionFD,&pRowA,sizeof(pRowA));
            fprintf(stderr,"Matrix A %dx%d\n",mColA,pRowA);

            double matrixA[MAX_SIZE_MATRIX][MAX_SIZE_MATRIX];
            matrixGenerator(matrixA,mColA, pRowA);
            printMatrix(matrixA,mColA,pRowA);
            write(connectionFD, &matrixA,sizeof(matrixA));

        }

    }
    fprintf(stderr,"\nServer shutting down...\n");
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
        return ERROR;
    sa.sin_family = (sa_family_t) hp->h_addrtype;
    sa.sin_port = htons(portNum);

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        return ERROR;
    if (bind(sock, (struct sockaddr *) &sa, sizeof(struct sockaddr_in)) < 0) {
        close(sock);
        return ERROR;
    }
    listen(sock, QUEUE_SIZE);
    return (sock);
}

int getConnection(int sock) {
    int connect;
    if ((connect = accept(sock,NULL,NULL)) < 0)
        return ERROR;
    return(connect);
}

void fireman(void) {
    while (waitpid(ERROR, NULL, WNOHANG) > 0) ;
}

void signalHandler(int sigNum) {

    if(sigNum == SIGINT){

        exitFlag = 1;
    }
}


void matrixGenerator(double matrix[][MAX_SIZE_MATRIX], int mCol, int pRow) {
    int i, j;
    int random;
//    matrix = allocateMatrix(mCol,pRow);
    for (i = 0; i < pRow; ++i) {
        for (j = 0; j < mCol; ++j) {
            random = rand() % 19 - 9;    /*-9 ile 9 arası*/
            matrix[i][j] = random;
            /*printf("%6.2f ", matrix[i][j]);*/

        }
        /*printf("\n");*/
    }

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
