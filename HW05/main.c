#include <sys/shm.h> // shared memory
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <wait.h>
#include <sys/time.h>
#include <sys/msg.h>
#include <sys/ipc.h>


#define ARGUMENTS 3
#define ERROR -1
#define TRUE 1
#define FALSE 0
#define BEGINNING 1
#ifndef PATH_MAX
#define PATH_MAX 255
#endif
#define LOGFILE "log.txt"
#define SEM_LOG_NAME "/myLog.sem"
#define SEM_SHM_NAME "/myShm.sem"
//for semaphore
#define PERMS (mode_t)(S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define FLAGS (O_CREAT | O_EXCL)


typedef struct {
    char string[PATH_MAX];
    char path[PATH_MAX];
} pthreadArg_t;

typedef struct {
    int totalNumOfStringFound;
    int numOfDirSearched;
    int numOfFileSearched;
    int numOfLineSearched;
    int numOfCascadeThreadCreated;
    int numOfSearchThreadCreated;
    int maxThreadRunConcurrent;
    double totalRunTimeInMiliSec;
} content_t;

/*  GLOBAL VARIABLES */

int exitCondition = 0;
key_t shmKey = 141044041;
int shmID;

int signalFlag = 0;

int numOfFilesAtCurrDir = 0;

int totalNumOfFile;

int totalNumOfDir;

int totalSearchedLine = 0;
sem_t semID;

// named semaphores
sem_t* semForLog;

sem_t* semForShared;



void signalHandler(int sigNumber);

int performSearch(const char *string, const char *pathName);

int explorePath(const char *string, const char *pathName);

int searchInFile(const char *findString, const char *fileName);

int findMatch(FILE *fp, const char *string);

FILE *createLogFile(FILE *logFP);

long getRunningTime(struct timeval timevalStart, struct timeval timevalEnd);

void countFilesAtCurrentDir(DIR *dirPtr, const char *path) ;

void countFileAndDir(DIR *dirPtr, const char *path, int *fNum, int *dNum) ;

int isDirectory(const char *pathName) ;

/* DERS KITABINDAN ALINDI - CH 14 - Semaphores */
int getnamed(char *name, sem_t **sem, int val) ;

void clearContent(content_t content) ;

void printContent(content_t content) ;

/* THREAD FUNCTION */
void *threadFunction(void *args) {


    int total;
//    printf("->now thread running!\n");
    pthreadArg_t *threadArg = (pthreadArg_t *) args;

//    printf("Thread file:%s\n", threadArg->path);
    sem_wait(&semID);
    char sharedName[PATH_MAX];
    //create unique shared memory
    sprintf(sharedName,"/%ld.sem",pthread_self()+getpid());
    getnamed(sharedName,&semForShared,1);
    sem_wait(semForLog);

    total = searchInFile(threadArg->string, threadArg->path);

    int *shmData = shmat(shmID,NULL,0);
    *shmData = (*shmData)+total;
    shmdt(shmData);
//    printf("ff: %d\n", total);
    sem_post(&semID);
    sem_post(semForLog);

    pthread_exit(NULL);
}

int main(int argc, const char *argv[]) {

    int total = 0;

    if (argc != ARGUMENTS) {
        printf("Usage: %s <string> <foldername>\n", argv[0]);
        exitCondition = ERROR;
        return ERROR;
    }

    struct sigaction sigact;
    sigemptyset(&sigact.sa_mask);
    sigact.sa_handler = signalHandler;
    sigact.sa_flags = 0;
    sigaction(SIGINT, &sigact, NULL);

    if(!signalFlag){
        total = performSearch(argv[1], argv[2]);
    }


    return 0;
}

int performSearch(const char *string, const char *pathName) {
    content_t content;
    struct timeval start;
    struct timeval end;
    gettimeofday(&start, NULL);

    getnamed(SEM_LOG_NAME,&semForLog,1);

    remove(LOGFILE);

    shmID = shmget(shmKey,sizeof(int),IPC_CREAT | 0666);

    int * shmData = shmat(shmID,NULL,0);
    if(shmData == (int *) ERROR){
        perror("shmat");
        exitCondition = ERROR;
        exit(TRUE);
    }

    *shmData = 0;


    printf("Searching for '%s' started in %s\n", string, pathName);

    int total = explorePath(string, pathName);
    gettimeofday(&end, NULL);

    long totalTime = getRunningTime(start, end);

    DIR* newDIRptr;
    if((newDIRptr = opendir(pathName)) != NULL){
        countFileAndDir(newDIRptr,pathName,&totalNumOfFile,&totalNumOfDir);
    }
    closedir(newDIRptr);

    clearContent(content);

    content.totalNumOfStringFound = total;
    content.numOfDirSearched += totalNumOfDir;
    content.numOfFileSearched += totalNumOfFile;
    content.numOfLineSearched = totalSearchedLine;
    content.totalRunTimeInMiliSec = totalTime;


   /* printf("num of file: %d\n",totalNumOfFile);
    printf("num of dir: %d\n",totalNumOfDir);
    printf("match total: %d\n",total);
    printf("Running time: %ld ms\n", totalTime);*/


    printContent(content);

    sem_close(semForLog);
    sem_unlink(SEM_LOG_NAME);
    sem_unlink(SEM_SHM_NAME);

    return total;
}

long getRunningTime(struct timeval timevalStart, struct timeval timevalEnd) {
    return (long) ((timevalEnd.tv_sec - timevalStart.tv_sec) * 1000.0f +
                   (timevalEnd.tv_usec - timevalStart.tv_usec) / 1000.0f);
}

int explorePath(const char *string, const char *pathName) {
    int i = 0;
    int j = 0;
    int total = 0;
    int totalMatches = 0;
    // int numOfFile = 0;
    // int numOfDir = 0;
    int status = 0;
    pid_t pid = getpid();
    pthread_t *threadIDs = NULL;
    pthreadArg_t *threadArgs;
    char path[PATH_MAX];
    struct dirent *dirEntPointer;
    DIR *direcPointer;
    if ((direcPointer = opendir(pathName)) == NULL) {
        perror("Failed to open directory");
        exitCondition = ERROR;
        return ERROR;
    }

    strcpy(path, pathName);


    numOfFilesAtCurrDir = 0;
    countFilesAtCurrentDir(direcPointer, pathName);
//    fprintf(stderr, "Num File: %d\n", numOfFilesAtCurrDir);
    rewinddir(direcPointer);

    if (numOfFilesAtCurrDir > 0) {
        threadIDs = (pthread_t *) calloc(numOfFilesAtCurrDir, sizeof(pthread_t));
        threadArgs = (pthreadArg_t *) calloc(numOfFilesAtCurrDir, sizeof(pthreadArg_t));
    }

    //count files and dirs
    DIR *copy;
    copy = direcPointer;
    countFileAndDir(copy, pathName, &totalNumOfFile, &totalNumOfDir);
    sem_init(&semID, 0, 1);

    rewinddir(direcPointer);
    i = 0;
    while ((dirEntPointer = readdir(direcPointer)) != NULL) {
        char currPath[PATH_MAX];
        strcpy(currPath, pathName);
        strcat(currPath, "/");
        strcat(currPath, dirEntPointer->d_name);

        //Directory check
        if (isDirectory(currPath)) {
            if (strcmp(dirEntPointer->d_name, ".") != 0 &&
                strcmp(dirEntPointer->d_name, "..") != 0) {
//                printf("PATH:%s\n->%s\n", currPath, dirEntPointer->d_name);
                //printf("Fork ==> dir\n" );


                if ((pid = fork()) < 0) {
                    perror("fork() failed.");
                    exitCondition = ERROR;
                    status = ERROR;
                }


                    //Child Process
                else if (0 == pid) {
                    //printf("child %d -> dir of %d\n",getpid(), getppid() );
                    if (numOfFilesAtCurrDir > 0) {
                        free(threadIDs);
                        free(threadArgs);
                    }

                    while ((closedir(direcPointer) == -1) && (errno == EINTR));
                    explorePath(string, currPath);
                    exit(1);
                }

            }

        } else {
//            printf("before thread creating\n");
//            printf("path: %s \n", currPath);

            /* Creating Thread */
            //sem_wait(&semID);
//            pthread_create((pthread_t *) &thread, 0, &threadFunction, &threadArgs);
//            pthread_join((pthread_t) thread, NULL);

            sprintf(threadArgs[i].string, "%s", string);
            sprintf(threadArgs[i].path, "%s", currPath);
            pthread_create(&threadIDs[i], 0, &threadFunction, &threadArgs[i]);
            ++i;
//            exit(1);
        }

    }
    char sharedName[PATH_MAX];
    //create unique shared memory
    if (numOfFilesAtCurrDir > 0) {
        for (j = 0; j < i; ++j) {
            sprintf(sharedName,"/%ld.sem",threadIDs[j]+getpid());
            sem_unlink(sharedName);
            pthread_join(threadIDs[j], NULL);
            //            printf("Thread join success!\n");
            }
    }

    //fprintf(stderr, "%d\n", status);
    while (wait(&status) > 0) {
        printf("Waiting processes...\n");

    }


    while ((closedir(direcPointer) == -1) && (errno == EINTR));

    if (numOfFilesAtCurrDir > 0) {
        free(threadIDs);
        free(threadArgs);
    }

    int *shmData = shmat(shmID,NULL,0);
//    printf("total: %d\n", *shmData);
    totalMatches = *shmData;
    return totalMatches;
}

int searchInFile(const char *findString, const char *fileName) {
//    printf("ThID: %lu PID: %d\n", pthread_self(), getpid());
//    printf("Scan file -> %s\n", fileName);
    /* VARIABLE DECLARATIONS */
    int stringSize = (int) strlen(findString);
    // Row and columns start from 1.
    int currentRow = BEGINNING;
    int currentColumn = BEGINNING;

    int NumOfMatches = 0;
    char currentChar;
    int prevLocation = 0; //Specify file location before calling findMatch(...)
    pthread_t thID = pthread_self();
    pid_t pid = getpid();

    FILE *logFileP = NULL;
    logFileP = createLogFile(logFileP);
    FILE *filePointer = fopen(fileName, "r");
    if (filePointer == NULL) {
        fprintf(stderr, "%s could NOT found.\n", fileName);
        exitCondition = ERROR;
        return ERROR;
    }

    currentChar = (char) getc(filePointer);

    while (currentChar != EOF) {

        if (currentChar == findString[0]) {
            if (stringSize == 1) {
                NumOfMatches++;
                fprintf(logFileP, "ThID: %lu PID: %d %s: [%d,%d] %s first character is found. \n",
                        thID, pid, fileName, currentRow, currentColumn, findString);
                fflush(logFileP);
                //sprintf(locationArr,"%d:%d\n",currentRow,currentColumn);
            } else {
                prevLocation = (int) ftell(filePointer);

                if (findMatch(filePointer, findString)) {
                    NumOfMatches++;
                    // sprintf(locationArr,"%d:%d",currentRow,currentColumn);
                    fprintf(logFileP,
                            "ThID: %lu PID: %d %s: [%d,%d] %s first character is found.\n",
                            thID, pid, fileName, currentRow, currentColumn, findString);
                    fflush(logFileP);
                }
                //return previous location at the file
                fseek(filePointer, prevLocation, SEEK_SET);
            }
            currentColumn++;
        } else if (currentChar == ' ' || currentChar == '\t') {
            currentColumn++;
        } else if (currentChar == '\n') {
            //go to head line
            ++currentRow;
            currentColumn = BEGINNING;
            ++totalSearchedLine;
        } else {
            ++currentColumn;
        }
        currentChar = (char) getc(filePointer);    //continue reading character
    }
    fclose(logFileP);
    fclose(filePointer);
    return NumOfMatches;
}

int findMatch(FILE *fp, const char *string) {

    char ch;
    int i = 1; // start from second character of string
    do {
        ch = (char) getc(fp);
        if (ch == string[i]) {
            ++i;
        } else if (ch == ' ' || ch == '\t' || ch == '\n') {/*ignore*/}
        else {
            return FALSE;
        }

    } while (i != strlen(string));

    return TRUE;
}

FILE *createLogFile(FILE *logFP) {

    char path[PATH_MAX];
    getcwd(path, PATH_MAX);
    strcat(path, "/");
    strcat(path, LOGFILE);    // log file create
    logFP = fopen(path, "a+");
    return logFP;
}

void signalHandler(int sigNumber) {
    exitCondition = sigNumber;
    printf("<==== Process[%d] SIGINT(^C) handled ! ====>\n\n", getpid());
    signalFlag = 1;
}


int isDirectory(const char *pathName) {
    struct stat statBuffer;

    if (stat(pathName, &statBuffer) != ERROR) {
        return S_ISDIR(statBuffer.st_mode);
    }

    return FALSE;
}

void countFilesAtCurrentDir(DIR *dirPtr, const char *path) {
    struct dirent *dirEntP = NULL;
    char newPath[PATH_MAX];


    while ((dirEntP = readdir(dirPtr)) != NULL) {
        sprintf(newPath, "%s/%s", path, dirEntP->d_name);
        if (!isDirectory(newPath)) {
            ++numOfFilesAtCurrDir;
        }
    }
}


void countFileAndDir(DIR *dirPtr, const char *path, int *fNum, int *dNum) {

    struct dirent *dirEntP = NULL;
    char newPath[PATH_MAX];
    char currPath[PATH_MAX];
    while ((dirEntP = readdir(dirPtr)) != NULL) {
        sprintf(newPath, "%s/%s", path, dirEntP->d_name);

        if (strcmp(dirEntP->d_name, ".") != 0 &&
            strcmp(dirEntP->d_name, "..") != 0) {
            if (isDirectory(newPath)) {
                ++(*dNum);
            } else {
                ++(*fNum);
            }
        }
    }

}


/* DERS KITABINDAN ALINDI - CH 14 - Semaphores */
int getnamed(char *name, sem_t **sem, int val) {
    while (((*sem = sem_open(name, FLAGS, PERMS, val)) == SEM_FAILED) &&
           (errno == EINTR)) ;
    if (*sem != SEM_FAILED)
        return 0;
    if (errno != EEXIST)
        return -1;
    while (((*sem = sem_open(name, 0)) == SEM_FAILED) && (errno == EINTR)) ;

    if (*sem != SEM_FAILED)
        return 0;

    return -1;
}

void printContent(content_t content) {
    printf("\n_Content_\n");
    printf("Total number of strings found: %d\n", content.totalNumOfStringFound);
    printf("Number of directories searched: %d\n", content.numOfDirSearched);
    printf("Number of files searched: %d\n", content.numOfFileSearched);
    printf("Number of lines searched: %d\n", content.numOfLineSearched);
    printf("Number of cascade threads created: %d\n", content.numOfFileSearched);
    printf("Number of search threads created: %d\n", content.numOfFileSearched);
    printf("Max # of threads running concurrently: %d\n", content.numOfFileSearched);
    printf("Number of Shared Memories: %d\n", content.numOfFileSearched);
    printf("Number of Size of Shared Memories: %d\n", content.numOfFileSearched);
    printf("Total run time, in milliseconds: %.3f\n", content.totalRunTimeInMiliSec);

    if(exitCondition == 0){
        printf("Exit Condition: normal\n");
    } else if(exitCondition == ERROR){
        printf("Exit Condition: ERROR no:%d\n", errno);
    } else{
        printf("Exit Condition: Signal received. SIG NUM:%d\n", exitCondition);

    }
}


void clearContent(content_t content) {

    content.totalNumOfStringFound = 0;
    content.numOfDirSearched = 0;
    content.numOfFileSearched = 0;
    content.numOfLineSearched = 0;
    content.numOfCascadeThreadCreated = 0;
    content.numOfSearchThreadCreated = 0;
    content.maxThreadRunConcurrent = 0;
    content.totalRunTimeInMiliSec = 0.0;

}