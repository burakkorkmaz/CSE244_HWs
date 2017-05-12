/*
 *      2016-2017 FALL
 *  CSE 244 SYSTEM PROGRAMMING
 *       #Homework 4
 *
 *  Created by Burak Kağan Korkmaz. 26.04.2017
 */

/* INCLUDES */
#include <unistd.h>    // fork, getpid
#include <dirent.h>    // opendir readdir
#include <errno.h>     // errno EINTR
#include <stdio.h>     // printf
#include <stdlib.h>    // malloc
#include <string.h>    // strlen, strcmp, strcpy, strcat
#include <sys/stat.h>    // stat, S_ISDIR
#include <sys/wait.h>   // wait()
#include <pthread.h>   // pthread_t and creating thread
#include <semaphore.h>   // semaphore initilize and
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>

/* CONSTANTS */

#define LOGFILE "log.log"
#define LOCATION_MAX 20 // location info string size
#ifndef PATH_MAX
#define PATH_MAX 255
#endif
#define BEGINNING 1
#define TRUE 1
#define FALSE 0
#define ERROR -1
#define FAIL -2
#define FIFONAME ".myfifo"
#define PERMISSIONS (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define FLAGS (O_CREAT | O_EXCL)
#define AMILLION 1000000


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

/* FUNCTION SIGNATURES */
/**
 * Searchs the string character by character in file and prints
 * columns and row of first found character
 * @param findString the String searched
 * @param fileName name of file which will be searched
 * @return Total Matches of the string in file
 */
int searchInFile(const char *findString, const char *fileName);

/**
 * Find a matches of string string after first character determined
 * @param fp The file pointer
 * @param string the String searched
 * @return 1 if matches, 0 otherwise
 */
int findMatch(FILE *fp, const char *string);

/**
 * explore in directory. Finds file and Folders
 * @param string the String searched
 * @param pathName The name of the file path
 * @return Total Matches of the string in file
 */
int explorePath(const char *string, const char *pathName);

/**
 * Specifies the path is whether file or directory
 * @param pathName Name of the Path
 * @return 1 if directory, 0 otherwise
 */
int isDirectory(const char *pathName);

/**
 *  Counts number of files and directories in a folder
 *  @param dirPtr Directory pointer
 *  @param path directory path
 *  @param fNum number of files
 *  @param dNum number of directories
 */
void countFileAndDir(DIR *dirPtr, const char *path, int *fNum, int *dNum);

/**
 * Creates or opens log file in append mode
 * @param logFP the log file pointer
 * @return Log file pointer
 */
FILE *createLogFile(FILE *logFP);

void printContent(content_t content);

void clearContent(content_t content);

/* GLOBAL VARIABLES */
int totalSearchedLine = 0;
int numOfFilesAtCurrDir = 0;
int totalNumOfFile = 0;
int totalNumOfDir = 0;

double runingTime = 0;
sem_t semID;

/* THREAD FUNCTION */
void *threadFunction(void *args) {
    sem_wait(&semID);
    pthreadArg_t threadArg = (*(pthreadArg_t *) args);
    searchInFile(threadArg.string, threadArg.path);
    sem_post(&semID);
    pthread_exit(NULL);
}

/* MAIN FUNCTION */
int main(int argc, char const *argv[]) {

    //Usage of 'listdir' command
    if (3 != argc) {
        printf("Usage: %s string <directory>\n", argv[0]);
        return ERROR;
    }

    content_t content;
    struct timeval start, end;

    pid_t pid = getpid();
    int totall = 0;
    remove(LOGFILE);
    clearContent(content);
    gettimeofday(&start, NULL);
    int total = explorePath(argv[1], argv[2]);
    gettimeofday(&end, NULL);
    runingTime = (end.tv_sec * AMILLION + end.tv_usec) - (start.tv_sec * AMILLION + start.tv_usec);


    char ch;
    FILE *log = NULL;
    log = createLogFile(log);
    while ((ch = (char) fgetc(log)) != EOF) {
        if (ch == '\n') {
            ++totall;
        }
    }
    fprintf(log, "Totally %d %s is found.\n", totall, argv[1]);
    printf("Totally %d %s is found.\n", totall, argv[1]);
    fclose(log);


    content.totalNumOfStringFound = totall;
    content.numOfDirSearched += totalNumOfDir;
    content.numOfFileSearched += totalNumOfFile;
    content.numOfLineSearched = totalSearchedLine;
    content.totalRunTimeInMiliSec = runingTime;
    printContent(content);


    return 0;
}

void printContent(content_t content) {
    printf("\n_Content_\n");
    printf("Total number of strings found: %d\n", content.totalNumOfStringFound);
    printf("Number of directories searched: %d\n", content.numOfDirSearched);
    printf("Number of files searched: %d\n", content.numOfFileSearched);
    printf("Number of lines searched: %d\n", content.numOfLineSearched);
    printf("Number of cascade threads created: %d\n", content.numOfCascadeThreadCreated);
    printf("Number of search threads created: %d\n", content.numOfSearchThreadCreated);
    printf("Max # of threads running concurrently: %d\n", content.maxThreadRunConcurrent);
    printf("Total run time, in milliseconds: %.3f\n", content.totalRunTimeInMiliSec);

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

int explorePath(const char *string, const char *pathName) {
    int i = 0;
    int j = 0;
    int total = 0;
    int totalMatches = 0;
    // int numOfFile = 0;
    // int numOfDir = 0;
    int status = 0;
    int fd[2];
    pid_t pid = getpid();
    pthread_t *threadIDs = NULL;
    pthread_t *thread;
    pthreadArg_t threadArgs;
    char path[PATH_MAX];
    char fifoname[PATH_MAX];
    struct dirent *dirEntPointer;
    DIR *direcPointer;
    if ((direcPointer = opendir(pathName)) == NULL) {
        perror("Failed to open directory");
        return ERROR;
    }

    strcpy(path, pathName);


    numOfFilesAtCurrDir = 0;
    countFilesAtCurrentDir(direcPointer, pathName);
    fprintf(stderr, "Num File: %d\n", numOfFilesAtCurrDir);
    rewinddir(direcPointer);
//    if(numOfFilesAtCurrDir > 0 ){
//        threadIDs = (pthread_t*)calloc(numOfFilesAtCurrDir,sizeof(pthread_t));
//    }

     //count files and dirs
    DIR * copy;
    copy = direcPointer;
     countFileAndDir(copy, pathName, &totalNumOfFile,&totalNumOfDir);


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
                strcmp(dirEntPointer->d_name, "..") != 0 ){
                //printf("PATH:%s\n->%s\n",currPath,  dirEntPointer->d_name);
                //printf("Fork ==> dir\n" );


               /* unlink(fifoname);
                unlink(FIFONAME);
                if (mkfifo(FIFONAME, PERMISSIONS) < 0) {
                    fprintf(stderr, "dir FIFO creating failed: %s\n", fifoname);
                    return ERROR;
                }*/

                if ((pid = fork()) < 0) {
                    perror("fork() failed.");
                    status = ERROR;
                }


                    //Child Process
                else if (0 == pid) {
                    //printf("child %d -> dir of %d\n",getpid(), getppid() );
                    while ((closedir(direcPointer) == -1) && (errno == EINTR));
                    explorePath(string, currPath);

                    exit(1);
                }

            }
        } else {


            //printf("PATH:%s\n->%s\n",currPath,  dirEntPointer->d_name);
            // pipeArr.pid = getpid();
            // if(pipe(fd) < 0){
            //     perror("Pipe failed!");
            //     return ERROR;




            sem_init(&semID, 0, 1);

            /* Creating Thread */
            sprintf(threadArgs.string, "%s", string);
            sprintf(threadArgs.path, "%s", currPath);
            pthread_create((pthread_t *) &thread, 0, &threadFunction, &threadArgs);
            pthread_join((pthread_t) thread, NULL);

//            pthread_create(&threadIDs[i],0,&threadFunction,&threadArgs);
//            ++i;
//            pthread_join((pthread_t) threadIDs[i], NULL);
//            sem_post(&semID);

//            exit(1);
        }
    }


    //fprintf(stderr, "%d\n", status);
    while (wait(&status) > 0) {
        printf("Waiting processes...\n");

        int temp = 0;
        close(fd[1]);
        while (read(fd[0], &temp, sizeof(int)) > 0) {
            //fprintf(stderr, "<-%d->\n", temp);
            totalMatches += temp;
            //fprintf(stderr, "TM:%d\n", totalMatches);
            //close(fd[0]);
        }

    }
    printf("i:%d\n", i);
//    if(numOfFilesAtCurrDir > 0)
//        for(j = 0; j < i; ++j){
//            pthread_join(threadIDs[i],NULL);
//        }
    //close(fd[0]);


    while ((closedir(direcPointer) == -1) && (errno == EINTR));
//    unlink(FIFONAME);
    free(threadIDs);
    return totalMatches;
}

int isDirectory(const char *pathName) {
    struct stat statBuffer;

    if (stat(pathName, &statBuffer) != ERROR) {
        return S_ISDIR(statBuffer.st_mode);
    }

    return FALSE;
}


FILE *createLogFile(FILE *logFP) {

    char path[PATH_MAX];
    getcwd(path, PATH_MAX);
    strcat(path, "/");
    strcat(path, LOGFILE);    // log file create
    logFP = fopen(path, "a+");
    return logFP;
}

int searchInFile(const char *findString, const char *fileName) {

    /* VARIABLE DECLARATIONS */
    int stringSize = (int) strlen(findString);
    // Row and columns start from 1.
    int currentRow = BEGINNING;
    int currentColumn = BEGINNING;
    int totalLineSearched = 0;
    int NumOfMatches = 0;
    char currentChar = '\0';
    int prevLocation = 0; //Specify file location before calling findMatch(...)
    char locationArr[LOCATION_MAX];
    int fifoFileDes = 0;
    FILE *logFileP = NULL;
    logFileP = createLogFile(logFileP);
    FILE *filePointer = fopen(fileName, "r");
    if (filePointer == NULL) {
        fprintf(stderr, "%s could NOT found.\n", fileName);
        return ERROR;
    }
    content_t content;
    pid_t PID = getpid();
    pthread_t threadID = pthread_self();
//    printf("PID:%d ThID:%lu\n", PID, threadID);
//    printf("test: |%s| |%s|\n", findString, fileName);




    currentChar = (char) getc(filePointer);
    //close(fileDes[0]);
    while (currentChar != EOF) {

        if (currentChar == findString[0]) {
            if (stringSize == 1) {
                NumOfMatches++;
                fprintf(logFileP, "PID:%d ThredID:%lu %s: [%d,%d] %s first character is found. \n",
                        PID, threadID, fileName, currentRow, currentColumn, findString);

                //sprintf(locationArr,"%d:%d\n",currentRow,currentColumn);
                //write(fileDes[1],locationArr,sizeof(locationArr));
            } else {
                prevLocation = (int) ftell(filePointer);

                if (findMatch(filePointer, findString)) {
                    NumOfMatches++;
                    // sprintf(locationArr,"%d:%d",currentRow,currentColumn);
                    // write(fileDes[1],locationArr,sizeof(locationArr));
                    fprintf(logFileP,
                            "PID:%d ThredID:%lu  %s: [%d,%d] %s first character is found.\n",
                            PID, threadID, fileName, currentRow, currentColumn, findString);
                }
                //return previous location at the file
                fseek(filePointer, prevLocation, SEEK_SET);
            }
            currentColumn++;
        } else if (currentChar == ' ' || currentChar == '\t') {
            currentColumn++;
        } else if (currentChar == '\n') {
            totalLineSearched++;
            //go to head line
            ++currentRow;
            currentColumn = BEGINNING;
        } else {
            ++currentColumn;
        }
        currentChar = (char) getc(filePointer);    //continue reading character

    }

    fclose(logFileP);
    fclose(filePointer);
    totalSearchedLine += totalLineSearched;
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
/* _END OF 141044041_HW4.c FILE_ */
