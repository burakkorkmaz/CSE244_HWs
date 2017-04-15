#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>




void swapRows(double **matrix, int nRow, int mRow);

double **matrixGenerator(int n) {
    int i, j, k;
    double **matrix = NULL;
    matrix = (double **) malloc((n) * sizeof(double *));
    if (matrix == NULL) {
        fprintf(stderr, "Matrix Allocation Error!\n");
        return NULL;
    }

    for (k = 0; k < n; ++k) {
        matrix[k] = (double *) malloc((n + 1) * sizeof(double));
    }
    //double arr = {10, 5}; //{0,-7 ,5 ,8, -1,  9, -3, 4,  7, -5, 5,  8, 3 , -4, 7,-9 };

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

double determinantOfMatrix(double **matrix, int n) {
    int i, j, k;
    double ratio;
    double det = 1;

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

    for (i = 0; i < n; i++)

        det *= matrix[i][i];

}


int main(int argc, char **argv) {
    srand(time(NULL));



    double **A;
    int n = atoi(argv[2]);
    A = matrixGenerator(n);

    printf("Det:%.2f\n", determinantOfMatrix(A, n));

    return 0;
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

void Transpose(double **a, int n) {
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

void CoFactor(double **a, int size, double **b) {
    int i, j, k, l, m, n;
    double det;
    double **c;

    c = malloc((size - 1) * sizeof(double *));
    for (i = 0; i < size - 1; i++)
        c[i] = malloc((size - 1) * sizeof(double));

    for (j = 0; j < size; j++) {
        for (i = 0; i < size; i++) {

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