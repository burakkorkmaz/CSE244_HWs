/*
 *      2016-2017 FALL
 *  CSE 244 SYSTEM PROGRAMMING
 *       #Homework 1
 *
 *  Created by Burak Kağan Korkmaz. 28.02.2017
 */

/* INCLUDES */
 #include <stdio.h>
 #include <string.h>

/* CONSTANTS */
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

/* MAIN FUNCTION */
int main(int argc, char const *argv[]) {
    //Usage of 'list' command
    if(3 != argc){
        printf("Usage: %s string filename\n",argv[0]);
        return ERROR;
    }

    printf("Totally %d '%s' matches FOUND.\n",
        searchInFile(argv[1], argv[2]), argv[1]);

  return 0;
}

int searchInFile(const char* findString, const char* fileName){
    /* VARIABLE DECLARATIONS */
    int stringSize = strlen(findString);
    // Row and columns start from 1.
    int currentRow = BEGINNING;
    int currentColumn = BEGINNING;

    int NumOfMatches = 0;
    char currentChar = '\0';
    int prevLocation = 0; //Specify file location before calling findMatch(...)

    FILE* filePointer = fopen(fileName,"r");
    if(filePointer == NULL){
        fprintf(stderr,"%s could NOT found.\n", fileName );
        return ERROR;
    }

    currentChar = getc(filePointer);
    //printf("%d -> |%c| at %d:%d\n",NumOfMatches, currentChar, currentRow,currentColumn);
    while (currentChar != EOF) {

        if(currentChar == findString[0]){
            if(stringSize == 1){
                NumOfMatches++;
                fprintf(stderr,"The first character found at [%d, %d]\n",
                    currentRow, currentColumn);
            }
            else{
                prevLocation = ftell(filePointer);

                if(findMatch(filePointer,findString)){
                    NumOfMatches++;
                    fprintf(stderr,"The first character found at [%d, %d]\n",
                        currentRow, currentColumn);
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
        currentChar = getc(filePointer);    //continue reading character
    }

    fclose(filePointer);
    return NumOfMatches;
}

int findMatch(FILE *fp,const char* string){

    char ch;
    int i = 1; // start from second character of string
    do{
        ch = getc(fp);
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
/* _END OF 141044041_HW1.c FILE_ */
