//
// Created by Burak Kağan Korkmaz on 30.05.2017.
//

#ifndef SHARED_AREA_H
#define SHARED_AREA_H

#define MAX_CLIENTS 200
#define ERROR -1
#define LOCALHOST "127.0.0.1"
#define MAX_SIZE_MATRIX 20
#define VECTOR 1

#define TRUE 1
#define FALSE 0
#define MAX_FILENAMES 255;

typedef struct{
    pid_t pid;
    int mCol;
    int pRow;
    int sockFD;
    /* MATRIX [COLUMN][ROW] */
    double matrixA[MAX_SIZE_MATRIX][MAX_SIZE_MATRIX];
    double matrixB[VECTOR][MAX_SIZE_MATRIX];
    double matrix_X1[VECTOR][MAX_SIZE_MATRIX];
    double matrix_X2[VECTOR][MAX_SIZE_MATRIX];
    double matrix_X3[VECTOR][MAX_SIZE_MATRIX];
    double error1;
    double error2;
    double error3;
}threadArg_t;

#endif //SHARED_AREA_H
