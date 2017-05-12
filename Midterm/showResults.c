#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define KERNEL_SIZE 3

void swapRows(double **matrix, int nRow, int mRow);

/* error return null*/
double **allocateMatrix(int n) {
    double **A;
    int i;
    A = (double **) malloc(n * sizeof(double *));
    if (!A) {
        perror("\nMatrix Allocation Error\n");
        return NULL;
    }

    for (i = 0; i < n; ++i) {
        A[i] = (double *) malloc(n * sizeof(double));
        if (!A[i]) {
            perror("\nMatrix Allocation Error\n");
            return NULL;
        }
    }
    return A;
}

void printMatrix(double **M, int n) {
    printf("\n");
    for (int i = 0; i < n; ++i) {
        printf("| ");
        for (int j = 0; j < n; ++j) {
            printf("%7.3f ", M[i][j]);
        }
        printf("|\n");
    }
    printf("\n");
}


double **matrixGenerator(int n) {
    int i, j, k;
    double **matrix = NULL;

    matrix = allocateMatrix(n);

    for (k = 0; k < n; ++k) {
        matrix[k] = (double *) malloc((n + 1) * sizeof(double));
    }

    srand(time(NULL));
    for (i = 0; i < n; ++i) {
        for (j = 0; j < n; ++j) {
            int random = rand() % 21 - 10;    //-10 ile 10 arası
            matrix[i][j] = random;
            printf("%6.2f ", matrix[i][j]);
        }
        printf("\n");
    }
    return matrix;
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

void coFactor(const double **a, int size, double **b) {
    int i, j, k, l, m, n;
    double det;
    double **c;

    double x = 0;
    double cof = 0;
    double cof2 = 0;


    c = malloc((size - 1) * sizeof(double *));
    for (i = 0; i < size - 1; i++)
        c[i] = malloc((size - 1) * sizeof(double));

    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            cof = a[i][j];
            /* Form the adjoint a_ij */
            m = 0;
            for (k = 0; k < size; k++) {
                if (k == i)
                    continue;
                n = 0;
                for (l = 0; l < size; l++) {
                    cof2 = a[k][l];
                    if (l == j)
                        continue;
                    c[m][n] = a[k][l];
                    x = a[k][l];
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


double **convolution(double **matrixOrig, int matrixSize) {
    int i, j, k, l, m, n, x, y;
    int kernelCenter = 0;
    double matrixKernel[3][3] = {{0,0,0},{0,1,0},{0,0,0}};
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
                        convolutionMatrix[i][j] += matrixOrig[x][y] * matrixKernel[k][l];
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
    double **firstMatrix = allocateMatrix(n);
    double **secondMatrix = allocateMatrix(n);
    double **thirdMatrix = allocateMatrix(n);
    double **lastMatrix = allocateMatrix(n);

    for (i = 0; i < matrixSize; ++i) {
        for (j = 0; j < matrixSize; ++j) {
            if (i < n && j < n) {
                firstMatrix[i][j] = matrixOriginal[i][j];
            }
            if (i < n && j >= n) {
                secondMatrix[i][j - n] = matrixOriginal[i][j];
            }
            if (i >= n && j < n) {
                thirdMatrix[i - n][j] = matrixOriginal[i][j];
            }
            if (i >= n && j >= n) {
                lastMatrix[i - n][j - n] = matrixOriginal[i][j];
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
                matrixOriginal[i][j] = firstMatrix[i][j];
            }
            if (i < n && j >= n) {
                matrixOriginal[i][j] = secondMatrix[i][j - n];
            }
            if (i >= n && j < n) {
                matrixOriginal[i][j] = thirdMatrix[i - n][j];
            }
            if (i >= n && j >= n) {
                matrixOriginal[i][j] = lastMatrix[i - n][j - n];
            }
        }
    }

    return matrixOriginal;

}

int main(int argc, char **argv) {

    double **A;
    double **B = NULL;
    int n = atoi(argv[1]) * 2;
    A = matrixGenerator(n);
    printf("Det:%.2f\n", determinantOfMatrix(A, n));
    B = inverseOfMatrix(A, n);
    if (B == NULL) {
        perror("inverse");
        return -1;
    }

    //printMatrix(inverseOfMatrix(E, 3), 3);

    /*for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            printf("%6.3f ", B[i][j]);
        }
        printf("\n");
    }*/




    double** C = convolution(A,n);

    printf("\nConvolution Matrix\n");
    printMatrix(C, n);




   /* printf("shitedinverse");

    A = shiftedInverse(A, n);
    printMatrix(A, n);*/
    return 0;
}
