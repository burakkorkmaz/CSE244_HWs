/*
 * Created by Burak Kağan Korkmaz on 13.04.2017.
 */


/*
 * LIBRARY INCLUDES
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <math.h>

/*
 * NAME CONSTANTS
 */
#define ARGUMENTS 2
#define ERROR -1
#define KERNEL_SIZE 3

/* Global Variables*/
int globalIntExitFlag = 0;
pid_t globalServerPid = 0;

/* FUNCTION PROTOTYPES */
static void signalHandler(int intSignalNumber);

double **allocateMatrix(int intSizeOfMatrix);

void printMatrix(double **M, int n);

void swapRows(double **matrix, int nRow, int size);

double determinantOfMatrix(double **dbPtrPtrA, int intSizeOfMatrix);

void transpose(double **a, int n);

void coFactor(double **a, int size, double **b);

double **inverseOfMatrix(double **A, int size);

double **convolution(double **matrixOrig, int matrixSize);

double **shiftedInverse(double **matrixOriginal, int matrixSize);


int main(int argc, char **argv) {
    int i, j;
    int intStatus = 0;
    int intMainFileDescriptor;
    int intNewServerDescriptor;
    pid_t pidClient = getpid();
    pid_t pidServer;
    int intSizeOfFifoName = 0;
    char *charPtrFifoName;
    int intMatrixSize = 0;
    double **dbPtrPtrM = NULL;
    double **dbPtrPtrTemp = NULL;
    double dbDeterminant;
    double dbResult1 = 0.0;
    double dbResult2 = 0.0;

    struct sigaction structSigAction;

    structSigAction.sa_handler = &signalHandler;


    if (argc != ARGUMENTS) {
        fprintf(stderr, "Usage: %s <main fifoname>\n", argv[0]);
        return ERROR;
    }

    if (sigaction(SIGINT, &structSigAction, NULL) < 0) {
        perror("sigaction -> SIGINT");
        return 1;
    }
    if (sigaction(SIGTERM, &structSigAction, NULL) < 0) {
        perror("sigaction-> SIGTERM");
        return 1;
    }

    if ((intMainFileDescriptor = open(argv[1], O_WRONLY)) == ERROR) {
        fprintf(stderr, "FIFO failed to open: %s\n", argv[1]);
        return ERROR;
    }
    intStatus = (int) write(intMainFileDescriptor, &pidClient, sizeof(pid_t));
    if (intStatus == ERROR) {
        fprintf(stderr, "FIFO failed to read: %s\n", argv[1]);
        return ERROR;

    }
    printf("Client PID: %d\n", pidClient);

    snprintf(NULL, (size_t) intSizeOfFifoName, "fifo_%d", pidClient);
    charPtrFifoName = (char *) malloc(intSizeOfFifoName * sizeof(char));
    sprintf(charPtrFifoName, "fifo_%d", pidClient);

    printf("fifoname:%s\n", charPtrFifoName);

    while ((intNewServerDescriptor = open(charPtrFifoName, O_RDONLY)) == ERROR);
    while (read(intNewServerDescriptor, &pidServer, sizeof(pid_t)) < 1);
    while (read(intNewServerDescriptor, &intMatrixSize, sizeof(int)) < 1);

    printf("Server PID: %d\n", pidServer);
    printf("Matrix Size: %d\n", intMatrixSize);
    globalServerPid = pidServer;

    dbPtrPtrM = allocateMatrix(intMatrixSize);

    for (i = 0; i < intMatrixSize; ++i) {
        for (j = 0; j < intMatrixSize; ++j) {
            while (read(intNewServerDescriptor, &dbPtrPtrM[i][j], sizeof(double)) < 1);
        }
    }

    printMatrix(dbPtrPtrM, intMatrixSize);

    while (!globalIntExitFlag) {

        printf("\nDeterminant: %f\n", (dbDeterminant = determinantOfMatrix(dbPtrPtrM, intMatrixSize)));

        if ((dbPtrPtrTemp = shiftedInverse(dbPtrPtrM, intMatrixSize)) != NULL) {
            dbResult1 = dbDeterminant - determinantOfMatrix(dbPtrPtrTemp, intMatrixSize);
        }

        dbResult2 = dbDeterminant - determinantOfMatrix(convolution(dbPtrPtrM, intMatrixSize), intMatrixSize);
        printf("Result1: %f\nResult2:%f\n", dbResult1, dbResult2);


        kill(pidServer, SIGUSR1);
        sleep(1);

        read(intNewServerDescriptor, &pidServer, sizeof(pid_t));
        read(intNewServerDescriptor, &intMatrixSize, sizeof(int));
        for (i = 0; i < intMatrixSize; ++i) {
            for (j = 0; j < intMatrixSize; ++j) {
                read(intNewServerDescriptor, &dbPtrPtrM[i][j], sizeof(double));
            }
        }
    }

    perror("exit");
    unlink(charPtrFifoName);
    free(charPtrFifoName);
    free(dbPtrPtrM);
    return 0;
}


double **allocateMatrix(int intSizeOfMatrix) {
    double **dbPtrPtrMatrix;
    int i;
    dbPtrPtrMatrix = (double **) malloc(intSizeOfMatrix * sizeof(double *));
    if (!dbPtrPtrMatrix) {
        perror("\nMatrix Allocation Error\n");
        return NULL;
    }

    for (i = 0; i < intSizeOfMatrix; ++i) {
        dbPtrPtrMatrix[i] = (double *) malloc(intSizeOfMatrix * sizeof(double));
        if (!dbPtrPtrMatrix[i]) {
            perror("\nMatrix Allocation Error\n");
            return NULL;
        }
    }
    return dbPtrPtrMatrix;
}

void printMatrix(double **M, int n) {
    int i, j;
    if (M == NULL)
        return;

    for (i = 0; i < n; ++i) {
        fprintf(stderr, "| ");
        for (j = 0; j < n; ++j) {
            fprintf(stderr, "%7.3f ", M[i][j]);
        }
        fprintf(stderr, "|\n");
    }
    fprintf(stderr, "\n");
}


static void signalHandler(int intSignalNumber) {

    printf("\nSIG NUMBER: %d\n", intSignalNumber);
    if (intSignalNumber == SIGINT) {
        fprintf(stderr, "Signal INTERRUPT has been received!\n");
        kill(globalServerPid, SIGINT);
        globalIntExitFlag = 1;
    }
    if (intSignalNumber == SIGTERM) {
        fprintf(stderr, "Signal TERMINATE has been received!\n");
        globalIntExitFlag = 1;
    }
}

void swapRows(double **matrix, int nRow, int size) {
    int i;
    double *temp;
    if (size - 1 == nRow) {
        return;
    }

    for (i = nRow; i < size - 1; ++i) {
        {
            if (matrix[nRow + 1][nRow + 1] != 0.0) {
                temp = matrix[nRow];
                matrix[nRow] = matrix[nRow + 1];
                matrix[nRow + 1] = temp;
                return;
            }
        }
    }
}

double determinantOfMatrix(double **A, int n) {
    int i, j, k;
    double ratio;
    double det = 1;
    double **matrix = allocateMatrix(n);
    /* copying the matrix */
    for (i = 0; i < n; ++i) {
        for (j = 0; j < n; ++j) {
            matrix[i][j] = A[i][j];
        }
    }
    /* apply gaussian with pivoting before determinant */
    for (k = 0; k < n; ++k) {
        if (matrix[k][k] == 0.0) {
            swapRows(matrix, k, n);
            det *= -1;
        }
        for (i = k + 1; i < n; ++i) {
            ratio = matrix[i][k] / matrix[k][k];

            for (j = k; j < n; ++j) {
                matrix[i][j] -= ratio * matrix[k][j];
            }
        }
    }
    /* Find determinant of Matrix */
    for (i = 0; i < n; i++)

        det *= matrix[i][i];

    return det;
}


/*  Transpose ve Cofactor fonksiyonlari icin
 *  https://www.cs.rochester.edu/~brown/Crypto/assts/projects/adj.html
 *  deki koddan yararlanılmıştır.
 */


void transpose(double **a, int n) {
    int i, j;
    double tmp;

    for (i = 1; i < n; i++) {
        for (j = 0; j < i; j++) {
            tmp = a[i][j];
            a[i][j] = a[j][i];
            a[j][i] = tmp;
        }
    }
}

void coFactor(double **a, int size, double **b) {
    int i, j, k, l, m, n;
    double det;
    double **c;

    c = malloc((size - 1) * sizeof(double *));
    for (i = 0; i < size - 1; i++)
        c[i] = malloc((size - 1) * sizeof(double));

    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            /* Form the adjoint a_ij */
            m = 0;
            for (k = 0; k < size; k++) {
                if (k == i)
                    continue;
                n = 0;
                for (l = 0; l < size; l++) {
                    if (l == j)
                        continue;
                    c[m][n] = a[k][l];
                    n++;
                }
                m++;
            }

            /* Calculate the determinate */
            det = determinantOfMatrix(c, size - 1);

            /* Fill in the elements of the cofactor */
            b[i][j] = pow(-1.0, i + j + 2.0) * det;
        }
    }
    for (i = 0; i < size - 1; i++)
        free(c[i]);
    free(c);
}

/* error return null*/
double **inverseOfMatrix(double **A, int size) {
    double **B;
    int i, j;
    double det = determinantOfMatrix(A, size);

    if (det == 0) {
        fprintf(stderr, "Determinant is zero, can not be inverse of Matrix");
        return NULL;
    }

    B = allocateMatrix(size);
    coFactor(A, size, B);
    transpose(B, size);

    for (i = 0; i < size; ++i) {
        for (j = 0; j < size; ++j) {
            B[i][j] /= det;
        }

    }
    return B;
}


/*
 *  http://www.songho.ca/dsp/convolution/convolution.html
 *  deki koddan yararlanılmıştır.
 */
double **convolution(double **matrixOrig, int matrixSize) {
    int i, j, k, l, m, n, x, y;
    int kernelCenter = 0;
    double matrixKernel[3][3] = {{0, 0, 0},
                                 {0, 1, 0},
                                 {0, 0, 0}};
    double skewSymmetric[3][3] = {{0, 1, -1},
                                 {-1, 0, -1},
                                 {1, 1, 0}};

    double matrixOther[3][3] = {{1, 1, -1},
                                {-1, 1, -1},
                                {1, 1, 1}};

    double **convolutionMatrix = allocateMatrix(matrixSize);

    // find center position of kernel (half of kernel size)
    kernelCenter = KERNEL_SIZE / 2;

    for (i = 0; i < matrixSize; ++i) {              // rows
        for (j = 0; j < matrixSize; ++j) {          // columns
            for (m = 0; m < KERNEL_SIZE; ++m) {    // kernel rows
                k = KERNEL_SIZE - 1 - m;      // row index of flipped kernel
                for (n = 0; n < KERNEL_SIZE; ++n) { // kernel columns
                    l = KERNEL_SIZE - 1 - n;  // column index of flipped kernel
                    // index of input signal, used for checking boundary
                    x = i + (m - kernelCenter);
                    y = j + (n - kernelCenter);
                    // ignore input samples which are out of bound
                    if (x >= 0 && x < matrixSize && y >= 0 && y < matrixSize)
                        convolutionMatrix[i][j] += matrixOrig[x][y] * skewSymmetric[k][l];
                }
            }
        }
    }
    return convolutionMatrix;
}

/* error return null */
double **shiftedInverse(double **matrixOriginal, int matrixSize) {
    int i, j;
    /* Reserved Size (n) of 2n Matrix */
    int n = matrixSize / 2;
    double **shiftedInverseMatrix = allocateMatrix(matrixSize);
    double **firstMatrix = allocateMatrix(n);
    double **secondMatrix = allocateMatrix(n);
    double **thirdMatrix = allocateMatrix(n);
    double **lastMatrix = allocateMatrix(n);

    for (i = 0; i < matrixSize; ++i) {
        for (j = 0; j < matrixSize; ++j) {
            shiftedInverseMatrix[i][j] = matrixOriginal[i][j];
        }
    }

    for (i = 0; i < matrixSize; ++i) {
        for (j = 0; j < matrixSize; ++j) {
            if (i < n && j < n) {
                firstMatrix[i][j] = shiftedInverseMatrix[i][j];
            }
            if (i < n && j >= n) {
                secondMatrix[i][j - n] = shiftedInverseMatrix[i][j];
            }
            if (i >= n && j < n) {
                thirdMatrix[i - n][j] = shiftedInverseMatrix[i][j];
            }
            if (i >= n && j >= n) {
                lastMatrix[i - n][j - n] = shiftedInverseMatrix[i][j];
            }
        }
    }

    if (determinantOfMatrix(firstMatrix, n) == 0.0 ||
        determinantOfMatrix(firstMatrix, n) == 0.0 ||
        determinantOfMatrix(firstMatrix, n) == 0.0 ||
        determinantOfMatrix(firstMatrix, n) == 0.0) {
        fprintf(stderr, "Error: Determinant is zero!\n"
                "During Shifted Inverse, One of reserved matrices couldn't be reversed.\n");
        return NULL;
    }

    firstMatrix = inverseOfMatrix(firstMatrix, n);
    secondMatrix = inverseOfMatrix(secondMatrix, n);
    thirdMatrix = inverseOfMatrix(thirdMatrix, n);
    lastMatrix = inverseOfMatrix(lastMatrix, n);


    for (i = 0; i < matrixSize; ++i) {
        for (j = 0; j < matrixSize; ++j) {
            if (i < n && j < n) {
                shiftedInverseMatrix[i][j] = firstMatrix[i][j];
            }
            if (i < n && j >= n) {
                shiftedInverseMatrix[i][j] = secondMatrix[i][j - n];
            }
            if (i >= n && j < n) {
                shiftedInverseMatrix[i][j] = thirdMatrix[i - n][j];
            }
            if (i >= n && j >= n) {
                shiftedInverseMatrix[i][j] = lastMatrix[i - n][j - n];
            }
        }
    }

    return shiftedInverseMatrix;

}