/*
 *      2016-2017 FALL
 *  CSE 244 SYSTEM PROGRAMMING
 *       #Homework 3
 *
 *  Created by Burak Kağan Korkmaz. 18.03.2017
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

/* CONSTANTS */
#define READ_FLAG (O_RDONLY)
#define WRITE_FLAG (O_WRONLY | O_APPEND | O_CREAT)
#define PERM_FIFO (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define LOGFILE "log.log"
#define LOCATION_MAX 20 // location info string size
#define FOUND_MAX 10    //Total Matches in a file
#ifndef PATH_MAX
 #define PATH_MAX 255
#endif
#define BEGINNING 1
#define TRUE 1
#define FALSE 0
#define ERROR -1
#define FD_SIZE 2
#define FAIL -2

//Process info for each process
typedef struct{
    pid_t pid;  // Process ID
    int fd[FD_SIZE];  // File Descipter for read and write
}process_t;

/* FUNCTION SIGNATURES */
/**
 * Searchs the string character by character in file and prints
 * columns and row of first found character
 * @param findString the String searched
 * @param fileName name of file which will be searched
 * @return Total Matches of the string in file
 */
int searchInFile(const char* findString,const char* fileName);

/**
 * Find a matches of string string after first character determined
 * @param fp The file pointer
 * @param string the String searched
 * @return 1 if matches, 0 otherwise
 */
int findMatch(FILE *fp, const char* string);

/**
 * explore in directory. Finds file and Folders
 * @param string the String searched
 * @param pathName The name of the file path
 * @return Total Matches of the string in file
 */
int explorePath(const char * string,const char * pathName);

/**
 * Specifies the path is whether file or directory
 * @param pathName Name of the Path
 * @return 1 if directory, 0 otherwise
 */
int isDirectory(const char * pathName);
/**
 *  Counts number of files and directories in a folder
 *  @param dirPtr Directory pointer
 *  @param path directory path
 *  @param fNum number of files
 *  @param dNum number of directories
 */
void countFileAndDir(DIR * dirPtr, const char * path, int * fNum, int * dNum);

/**
 * Creates or opens log file in append mode
 * @param logFP the log file pointer
 * @return Log file pointer
 */
FILE * createLogFile(FILE * logFP);

/* MAIN FUNCTION */
int main(int argc, char const *argv[]) {

    //Usage of 'listdir' command
    if(3 != argc){
        printf("Usage: %s string <directory>\n",argv[0]);
        return ERROR;
    }

    pid_t pid = getpid();
    int totall = 0;
    remove(LOGFILE);
    int total = explorePath(argv[1],argv[2]);


    if(pid > 0){
        char ch;
        FILE *log = NULL;
        log = createLogFile(log);
        while((ch = (char) fgetc(log)) != EOF){
            if(ch == '\n'){
                ++totall;
            }
        }
        fprintf(log,"Totally %d %s is found.\n",totall, argv[1]);
        printf("Totally %d %s is found.\n",totall, argv[1]);
        fclose(log);

    }

  return 0;
}

int explorePath(const char * string,const char * pathName){

    int total = 0;
    int totalMatches = 0;
    // int numOfFile = 0;
    // int numOfDir = 0;
    int status = 0;
    int fd[2];
    pid_t pid = getpid();

    // process_t * pipeArr = NULL;
    // process_t * fifoArr = NULL;

    struct dirent * dirEntPointer;
    DIR * direcPointer;
    if((direcPointer = opendir(pathName)) == NULL){
        perror ("Failed to open directory");
        return ERROR;
    }



    char path[PATH_MAX];
    strcpy(path, pathName);

    // DIR * copyDPtr = direcPointer;
    // //count process
    // countFileAndDir(copyDPtr, pathName, &numOfFile, &numOfDir);
    char fifoname[] = "fifo.fifo";
    // if(numOfFile > 0){
    //     pipeArr = (process_t)malloc(sizeof(process_t) * numOfFile);
    // }
    // if(numOfDir > 0){
    //     fifoArr = (process_t)malloc(sizeof(process_t) * numOfDir);
    // }




    //PIPE CREATING
    if(pipe(fd) < 0){
        perror("Pipe failed!");
        return ERROR;
    }

    while ((dirEntPointer = readdir(direcPointer)) != NULL){
        char currPath[PATH_MAX];
        strcpy(currPath,pathName);
        strcat(currPath,"/");
        strcat(currPath,dirEntPointer->d_name);
        //Directory check
        if(isDirectory(currPath)){
            if(strcmp(dirEntPointer->d_name ,".") != 0 &&
                strcmp(dirEntPointer->d_name ,"..") != 0){
                    //printf("PATH:%s\n->%s\n",currPath,  dirEntPointer->d_name);
                    //printf("Fork ==> dir\n" );

                if( mkfifo(fifoname,PERM_FIFO) == FAIL){
                    perror("Fifo failed.");
                    status = ERROR;
                }


                if((pid = fork()) < 0){
                    perror("fork() failed.");
                    status = ERROR;
                }


                //Child Process
                else if(0 == pid){
                    //printf("child %d -> dir of %d\n",getpid(), getppid() );
                    while ((closedir(direcPointer) == -1) && (errno == EINTR));
                    explorePath(string, currPath);
                    unlink(fifoname);
                    exit(1);
                }

            }
        }
        else{

            //printf("PATH:%s\n->%s\n",currPath,  dirEntPointer->d_name);
            // pipeArr.pid = getpid();
            // if(pipe(fd) < 0){
            //     perror("Pipe failed!");
            //     return ERROR;
            // }

            //Fork the process
            if((pid = fork()) < 0){
                perror("fork() failed.");
                status = ERROR;
            }
            //Child Process
            else if(0 == pid){
                close(fd[0]);
                while ((closedir(direcPointer) == -1) && (errno == EINTR));
//pipe total matches in a file
                total = searchInFile(string, currPath);
                //fprintf(stderr, "child T:%d\n", total);
                close(fd[0]);
                write(fd[1], &total, sizeof(int));
                //fprintf(stderr, "%s\n", "send to pipe");
                close(fd[1]);
                exit(status);
            }
            //Parent Process
            else{

                //close(fd[1]);
            }
        }
    }

    //fprintf(stderr, "%d\n", status);
    while(wait(&status) > 0){
        printf("Waiting processes...\n" );
        int temp = 0;
        close(fd[1]);
        while(read(fd[0],&temp , sizeof(int))>0){
            //fprintf(stderr, "<-%d->\n", temp);
            totalMatches += temp;
            //fprintf(stderr, "TM:%d\n", totalMatches);
            //close(fd[0]);
        }

    }


    //close(fd[0]);

    while ((closedir(direcPointer) == -1) && (errno == EINTR)) ;
    return totalMatches;
}

int isDirectory(const char * pathName){
    struct stat statBuffer;

    if(stat(pathName, &statBuffer) != ERROR){
        return S_ISDIR(statBuffer.st_mode);
    }

    return FALSE;
}

void countFileAndDir(DIR * dirPtr, const char * path, int * fNum, int * dNum){

    struct dirent * dirEntP = NULL;
    char newPath[PATH_MAX];


    while ((dirEntP = readdir(dirPtr)) != NULL) {
        sprintf(newPath, "%s/%s", path,dirEntP->d_name);

        if(strcmp(dirEntP->d_name ,".") != 0 &&
            strcmp(dirEntP->d_name ,"..") != 0)
        {
            if(isDirectory(newPath)){
                ++(*dNum);
            }
            else{
                ++(*fNum);
            }
        }
    }

}


FILE * createLogFile(FILE * logFP){

    char path[PATH_MAX];
    getcwd(path,PATH_MAX);
    strcat(path,"/");
    strcat(path,LOGFILE);    // log file create
    logFP = fopen(path,"a+");
    return logFP;
}

int searchInFile(const char* findString, const char* fileName){
    /* VARIABLE DECLARATIONS */
    int stringSize = (int) strlen(findString);
    // Row and columns start from 1.
    int currentRow = BEGINNING;
    int currentColumn = BEGINNING;

    int NumOfMatches = 0;
    char currentChar = '\0';
    int prevLocation = 0; //Specify file location before calling findMatch(...)

    char locationArr[LOCATION_MAX];

    FILE* logFileP = NULL;
    logFileP = createLogFile(logFileP);
    FILE* filePointer = fopen(fileName,"r");
    if(filePointer == NULL){
        fprintf(stderr,"%s could NOT found.\n", fileName );
        return ERROR;
    }

    currentChar = (char) getc(filePointer);
    //close(fileDes[0]);
    while (currentChar != EOF) {

        if(currentChar == findString[0]){
            if(stringSize == 1){
                NumOfMatches++;
                fprintf(logFileP,"%s: [%d,%d] %s first character is found. \n",
                    fileName, currentRow, currentColumn, findString);

                    //sprintf(locationArr,"%d:%d\n",currentRow,currentColumn);
                    //write(fileDes[1],locationArr,sizeof(locationArr));
            }
            else{
                prevLocation = (int) ftell(filePointer);

                if(findMatch(filePointer,findString)){
                    NumOfMatches++;
                    // sprintf(locationArr,"%d:%d",currentRow,currentColumn);
                    // write(fileDes[1],locationArr,sizeof(locationArr));
                    fprintf(logFileP,
                        "%s: [%d,%d] %s first character is found.\n",
                        fileName, currentRow, currentColumn, findString);
                }
                //return previous location at the file
                fseek(filePointer, prevLocation, SEEK_SET);
            }
            currentColumn++;
        }
        else if(currentChar == ' ' || currentChar == '\t'){
            currentColumn++;
        }
        else if(currentChar == '\n'){
            //go to head line
            ++currentRow;
            currentColumn = BEGINNING;
        }
        else{
            ++currentColumn;
        }
        currentChar = (char) getc(filePointer);    //continue reading character
    }
    fclose(logFileP);
    fclose(filePointer);
    return NumOfMatches;
}

int findMatch(FILE *fp, const char* string){

    char ch;
    int i = 1; // start from second character of string
    do{
        ch = (char) getc(fp);
        if(ch == string[i]){
            ++i;
        }
        else if(ch == ' ' || ch == '\t' || ch == '\n'){/*ignore*/}
        else{
            return FALSE;
        }

    }while(i != strlen(string));

    return TRUE;
}
/* _END OF 141044041_HW3.c FILE_ */
