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

#define ARGUMENTS 3
#define ERROR -1
#define TRUE 1
#define FALSE 0
#define BEGINNING 1
#ifndef PATH_MAX
#define PATH_MAX 255
#endif
#define LOGFILE "log.txt"

typedef struct {
    char string[PATH_MAX];
    char path[PATH_MAX];
} pthreadArg_t;

/*  GLOBAL VARIABLES */
int numOfFilesAtCurrDir;

int totalNumOfFile;

sem_t semID;

int totalNumOfDir;

int searchInFile(const char *findString, const char *fileName);

int findMatch(FILE *fp, const char *string);


FILE *createLogFile(FILE *logFP);

int explorePath(const char *string, const char *pathName);

/* THREAD FUNCTION */
void *threadFunction(void *args) {



    int x;
    printf("->now thread running!\n");
    pthreadArg_t *threadArg = (pthreadArg_t *) args;

    printf("Thread file:%s\n",threadArg->path);
    sem_wait(&semID);
    x = searchInFile(threadArg->string, threadArg->path);
    printf("ff: %d\n", x);


    sem_post(&semID);
    pthread_exit(NULL);
}

int main(int argc, const char *argv[]) {
    if (argc != ARGUMENTS) {
        printf("Usage: %s <string> <foldername>\n", argv[0]);
        return ERROR;
    }
    remove(LOGFILE);

    printf("found: %d\n", explorePath(argv[1], argv[2]));


    return 0;
}

int searchInFile(const char *findString, const char *fileName) {
    printf("ThID: %lu PID: %d\n", pthread_self(), getpid());
    printf("Scan file -> %s\n",fileName);
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
    pid_t pid = getpid();
    pthread_t *threadIDs = NULL;
    pthreadArg_t *threadArgs;
    char path[PATH_MAX];
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

    if (numOfFilesAtCurrDir > 0) {
        threadIDs = (pthread_t *) calloc(numOfFilesAtCurrDir, sizeof(pthread_t));
        threadArgs = (pthreadArg_t*)calloc(numOfFilesAtCurrDir, sizeof(pthreadArg_t));
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
                    printf("PATH:%s\n->%s\n",currPath,  dirEntPointer->d_name);
                    //printf("Fork ==> dir\n" );


                    if ((pid = fork()) < 0) {
                        perror("fork() failed.");
                        status = ERROR;
                    }


                        //Child Process
                    else if (0 == pid) {
                        //printf("child %d -> dir of %d\n",getpid(), getppid() );
                        if(numOfFilesAtCurrDir > 0){
                            free(threadIDs);
                            free(threadArgs);
                        }

                        while ((closedir(direcPointer) == -1) && (errno == EINTR));
                        explorePath(string, currPath);
                        exit(1);
                    }

                }

        } else {
            printf("before thread creating\n");
            printf("path: %s \n", currPath);

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

    if (numOfFilesAtCurrDir > 0)
        for (j = 0; j < i; ++j) {
            pthread_join(threadIDs[j], NULL);
//            printf("Thread join success!\n");
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

    return totalMatches;
}