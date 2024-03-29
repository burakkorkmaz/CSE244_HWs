/*
 *      2016-2017 FALL
 *  CSE 244 SYSTEM PROGRAMMING
 *       #Homework 2
 *
 *  Created by Burak Kağan Korkmaz. 08.03.2017
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
#ifndef PATH_MAX
 #define PATH_MAX 255
#endif
#define BEGINNING 1
#define TRUE 1
#define FALSE 0
#define ERROR -1

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

FILE * createLogFile(FILE * logFP);

/* MAIN FUNCTION */
int main(int argc, char const *argv[]) {

    pid_t pid = getpid();
    int counter = 0;
    //Usage of 'listdir' command
    if(3 != argc){
        printf("Usage: %s string <directory>\n",argv[0]);
        return ERROR;
    }

    remove("log.log");
    printf("Total: %d\n",explorePath(argv[1],argv[2]));
        //searchInFile(argv[1], argv[2]), argv[1]);


    if(pid > 0){
        char ch;
        FILE *log = NULL;
        log = createLogFile(log);
        while((ch = (char) fgetc(log)) != EOF){
            if(ch == '\n'){
                ++counter;
            }
        }
        fprintf(log,"Totally %d %s is found.\n",counter,argv[1]);
        printf("Totally %d %s is found.\n",counter,argv[1]);
        fclose(log);

    }

  return 0;
}

int explorePath(const char * string,const char * pathName){

    char path[PATH_MAX];
    strcpy(path, pathName);
    int totalMatches = 0;
    pid_t pid = getpid();
    int status = 0;

    struct dirent * dirEntPointer;
    DIR * direcPointer;
    if((direcPointer = opendir(pathName)) == NULL){
        perror ("Failed to open directory");
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
                if((pid = fork()) < 0){
                    perror("fork() failed.");
                    status = ERROR;
                }
                //Child Process
                else if(0 == pid){
                    //printf("child %d -> dir of %d\n",getpid(), getppid() );
                    while ((closedir(direcPointer) == -1) && (errno == EINTR));
                    explorePath(string, currPath);
                    exit(1);
                }

            }
        }
        else{

            if((pid = fork()) < 0){
                perror("fork() failed.");
                status = ERROR;
            }
            //Child Process
            else if(0 == pid){
                while ((closedir(direcPointer) == -1) && (errno == EINTR));
                totalMatches = searchInFile(string,currPath);
                //printf("child %d of %d ->file  match:%d\n",getpid(), getppid(), totalMatches);
                exit(totalMatches);
            }
        }
    }
    while(wait(&status) > 0){
        printf("Waiting processes...\n" );
    }


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

FILE * createLogFile(FILE * logFP){

    char path[PATH_MAX];
    getcwd(path,PATH_MAX);
    strcat(path,"/log.log");    // log file create
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

    FILE* logFileP = NULL;
    logFileP = createLogFile(logFileP);
    FILE* filePointer = fopen(fileName,"r");
    if(filePointer == NULL){
        fprintf(stderr,"%s could NOT found.\n", fileName );
        return ERROR;
    }

    currentChar = (char) getc(filePointer);
    while (currentChar != EOF) {

        if(currentChar == findString[0]){
            if(stringSize == 1){
                NumOfMatches++;
                fprintf(logFileP,"%s: [%d,%d] %s first character is found. \n",
                    fileName, currentRow, currentColumn, findString);
            }
            else{
                prevLocation = (int) ftell(filePointer);

                if(findMatch(filePointer,findString)){
                    NumOfMatches++;
                    fprintf(logFileP,"%s: [%d,%d] %s first character is found.\n",
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

int findMatch(FILE *fp,const char* string){

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
/* _END OF 141044041_HW2.c FILE_ */
