/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */

char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
	/* 
	 * s = 5 , (S = 32) , E = 1, b = 5
	 * B = 2^5 = 32 = int * 8
	 * 32 / 32
	 * 64 / 64
	 * 61 / 67
	 */
	int i,j,k,m;
	int t0,t1,t2,t3,t4,t5,t6,t7;
	if ( M == 32 && N == 32 ){
		for(j=0; j <M; j+=8){
			for(i = 0; i < N; i++){
				t0 = A[i][ j ];
				t1 = A[i][j+1];
				t2 = A[i][j+2];
				t3 = A[i][j+3];
				t4 = A[i][j+4];
				t5 = A[i][j+5];
				t6 = A[i][j+6];
				t7 = A[i][j+7];

				// many miss
				B[ j ][i] = t0;
				B[j+1][i] = t1;
				B[j+2][i] = t2;	
				B[j+3][i] = t3;
				B[j+4][i] = t4;
				B[j+5][i] = t5;
				B[j+6][i] = t6;
				B[j+7][i] = t7;
			}
		}
	}
	else if( M == 64 && N == 64 ){
		// 4×4
		for( i=0; i< N; i+=4){
			for( j=0; j< M;j+=4){
				for(k =i;k<i+4;k+=2){
					// 2×4 -> 2 round
					t0 = A[k][j];
					t1 = A[k][j+1];
					t2 = A[k][j+2];
					t3 = A[k][j+3];
					t4 = A[k+1][j];
					t5 = A[k+1][j+1];
					t6 = A[k+1][j+2];
					t7 = A[k+1][j+3];

					B[j][k]=t0;
					B[j+1][k]=t1;
					B[j+2][k]=t2;
					B[j+3][k]=t3;
					B[j][k+1]=t4;
					B[j+1][k+1]=t5;
					B[j+2][k+1]=t6;
					B[j+3][k+1]=t7;
				}
			}
		}
	}
	else{//61 / 67
		// 16*16
		for(i = 0; i<N; i+=16){
			for(j = 0; j<M; j+=16){
				for(k = i;k< i+16 && k<N;k++){
					for( m=j;m<j+16 &&m<M;m++){
						B[m][k]= A[k][m];
					}
				}
			}
		}
	}
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

