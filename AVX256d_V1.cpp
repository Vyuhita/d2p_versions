#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <immintrin.h>

#define TOL 1e-4

double* a;
double* b;
double* c;
int N;
int block_size; // Added block size parameter

// Declare MatMul_AVX function
void MatMul_AVX(double* a, double* b, double* c, int srA, int scA, int srB, int scB, int srC, int scC, int dim);

void printMatrix(double* mat, int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            printf("%8.2f ", mat[i * cols + j]);
        }
        printf("\n");
    }
    printf("\n");
}

void Recursive_MatMul(double* a, double* b, double* c, int srA, int scA, int srB, int scB, int srC, int scC, int dim) {
    int newDim = dim / 2;

    if (dim <= block_size) {
        MatMul_AVX(a, b, c, srA, scA, srB, scB, srC, scC, dim); // Use AVX for the base case
        return;
    }

    Recursive_MatMul(a, b, c, srA, scA, srB, scB, srC, scC, newDim);
    Recursive_MatMul(a, b, c, srA, scA + newDim, srB + newDim, scB, srC, scC, newDim);
    Recursive_MatMul(a, b, c, srA, scA, srB, scB + newDim, srC, scC + newDim, newDim);
    Recursive_MatMul(a, b, c, srA, scA + newDim, srB + newDim, scB + newDim, srC, scC + newDim, newDim);
    Recursive_MatMul(a, b, c, srA + newDim, scA, srB, scB, srC + newDim, scC, newDim);
    Recursive_MatMul(a, b, c, srA + newDim, scA + newDim, srB + newDim, scB, srC + newDim, scC, newDim);
    Recursive_MatMul(a, b, c, srA + newDim, scA, srB, scB + newDim, srC + newDim, scC + newDim, newDim);
    Recursive_MatMul(a, b, c, srA + newDim, scA + newDim, srB + newDim, scB + newDim, srC + newDim, scC + newDim, newDim);
}

void MatMul_AVX(double* a, double* b, double* c, int srA, int scA, int srB, int scB, int srC, int scC, int dim) {
    int i, j, k;
    __m256d va, vb, vc;
    double result[4];  // Store the result elements

    for (i = 0; i < dim; i++) {
        for (j = 0; j < dim; j++) {
            vc = _mm256_setzero_pd();  // Initialize the result vector to zeros
            for (k = 0; k < dim; k += 4) {
                va = _mm256_loadu_pd(&a[(i + srA) * N + scA + k]);  // Load 4 double-precision values from matrix 'a'
                vb = _mm256_loadu_pd(&b[(srB + k) * N + j + scB]);  // Load 4 double-precision values from matrix 'b'
                //vc = _mm256_add_pd(vc, _mm256_mul_pd(va, vb));      // Multiply and accumulate
		vc = _mm256_fmadd_pd(va, vb, vc);
            }
            // Extract and accumulate the result into result[]
            //_mm256_storeu_pd(result, vc);
            double sum = 0.0;
            for (int idx = 0; idx < 4; idx++) {
                sum += vc[idx];
            }
            // Store the accumulated result in the matrix 'c'
            c[(i + srC) * N + j + scC] += sum;
        }
    }
}

void Init() {
    a = (double*)malloc(N * N * sizeof(double));
    b = (double*)malloc(N * N * sizeof(double));
    c = (double*)malloc(N * N * sizeof(double));

    if (a == NULL || b == NULL || c == NULL) {
        printf("Error: can't allocate memory for matrices.\n\n");
        exit(1);
    }

    srand(time(NULL));

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            a[i * N + j] = 1.0; // Initialize with double values
            b[i * N + j] = 1.0; // Initialize with double values
        }
    }

    /*printf("Matrix A:\n");
    printMatrix(a, N, N);

    printf("Matrix B:\n");
    printMatrix(b, N, N);*/
}

void FreeMemory() {
    free(a);
    free(b);
    free(c);
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Usage: %s N block_size\n", argv[0]);
        exit(1);
    }

    N = atoi(argv[1]);
    block_size = atoi(argv[2]);

    int threshold = N / 2;

    if (threshold < block_size) {
        printf("Block size should be smaller than or equal to N/2.\n");
        exit(1);
    }

    Init();

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            c[i * N + j] = 0.0;
        }
    }

    clock_t start, end;
    start = clock();
    Recursive_MatMul(a, b, c, 0, 0, 0, 0, 0, 0, N);
    end = clock();
    double total_time = ((double)(end - start)) / CLOCKS_PER_SEC;

     //printf("Matrix C (Result):\n");
   //  printMatrix(c, N, N);

    printf("Elapsed time for recursive_matmul execution = %f seconds.\n", total_time);

    FreeMemory();

    return 0;
}

