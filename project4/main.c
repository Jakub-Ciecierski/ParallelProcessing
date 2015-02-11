#define _GNU_SOURCE 
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "mpi.h"
/*
 *******************************************************************
 * An upper triangular (all elements bellow main diagonal are zero)
 * dense (most of the elements are nonzero) 
 * matrix is split by columns on N processors.
 * Implement the backward substitution. Analyze the speed-up.
 *******************************************************************
 */
 
 /*
  * Data struct for each process
  */
 typedef struct 
 {
	int rank;
	int size;
	// indicates first and last column in his possession
	int first_col;
	int last_col;

	// count of columns
	int count;

	/*
	 * Pointer to elements in columns he is holding
	 * A[i,i] = c[(i-first)*n + i]
	 * where n is the dim of A
	 */
	double* c;
 } worker_struct;

/*
 * Psudo code of sequential algorithm 
 * for i:n-1 to 1
 * 		x[i] = b[i] / a[i,i]
 * 		for j: 0 to i-1
 * 			b[j] = b[j] - x[i]*a[j,i]
 * 		end
 * end
 * */ 
void backward_substitution(worker_struct worker, double* b, double* x, int n)
{
	MPI_Status stat;
	printf("I'm %d worker my data is: \n" , worker.rank);
	printf("first_col: %d, last_col: %d, count: %d \n" , worker.first_col, worker.last_col, worker.count);
	
	// if it is not the last process
	if(worker.rank != worker.size -1)
	{
		// wait for the previous worker to update vectors b and x
		printf("Worker#%d before Recv\n" , worker.rank);
		MPI_Recv(b, n, MPI_DOUBLE, worker.rank + 1, 0, MPI_COMM_WORLD, &stat);
		MPI_Recv(x, n, MPI_DOUBLE, worker.rank + 1, 0, MPI_COMM_WORLD, &stat);
		printf("Worker#%d after Recv\n" , worker.rank);
	}

	// start the global_i-th iteration
	// update b and send it to next worker
	// iterate through all columns that this worker possesses
	printf("Worker#%d Starting work \n" , worker.rank);
	int i = 0;
	for(i = 0; i < worker.count; i++)
	{
		// represents the real column index of matrix A
		int global_i = worker.last_col - i;

		// print the column he is working on
		printf("W#%d Column: %d \n" , worker.rank, global_i);
		int t = 0;
		for(t = 0; t < n; t++)
		{
			double value = worker.c[(global_i - worker.first_col)*n + t];
			printf("W#%d A[%d][%d]: %lf \n" , worker.rank, t, global_i, value);
		}
		printf("\n");
		
		// compute x[global_i]
		x[global_i] = b[global_i] / worker.c[(global_i - worker.first_col)*n + global_i];
		
		// print status
		double a_val = worker.c[(global_i - worker.first_col)*n + global_i];
		double b_val = b[global_i];
		printf("W#%d x[%d] = %lf, b[%d] = %lf, A[%d][%d] = %lf \n", worker.rank, global_i, x[global_i],global_i,b[global_i], global_i,global_i, a_val);
		printf("\n");
		
		// update b
		int j = 0;
		for(j = 0; j < global_i; j++)
		{
			double value = worker.c[(global_i - worker.first_col)*n + j];
			printf("W#%d b[%d](%lf) - x[%d](%lf) * A[%d][%d](%lf)  \n" , worker.rank, j, b[j], global_i, x[global_i], j, global_i, value);
			b[j] = b[j] - x[global_i] * worker.c[(global_i - worker.first_col)*n + j];
		}
		printf("\n");
		
		// print current b
		for(t = 0; t < n; t++)
		{
			double value = b[t];
			printf("W#%d b[%d]: %lf \n" , worker.rank, t, value);
		}
		printf("\n");

	}
	// his work is done, pass the updated b and x vectors to next worker
	if (worker.rank != 0)
	{
		printf("W#%d Sending b \n" , worker.rank);
		MPI_Send(b, n, MPI_DOUBLE, worker.rank - 1, 0, MPI_COMM_WORLD);
		MPI_Send(x, n, MPI_DOUBLE, worker.rank - 1, 0, MPI_COMM_WORLD);
		printf("W#%d After Sending b \n" , worker.rank);
	}

}

int main(int argc, char* argv[])
{
	int rank, size;

	// Initialize the environment
	MPI_Init( &argc, &argv );
	MPI_Comm_rank( MPI_COMM_WORLD, &rank );
	MPI_Comm_size( MPI_COMM_WORLD, &size );

	// example of big matrix in column-major order
	int n = 100;
	double* A = (double*)malloc(sizeof(double)*n*n);
	int r = 0;
	int c = 0;
	double val = 1;
	for(c = 0; c < n; c++)
	{
		for(r = 0; r < n; r++)
		{
			if(r > c)
				A[c*n + r] = 0;
			else
			{
				A[c*n + r] = val;
				val++;
			}		
		}
	}	

	double* b = (double*)malloc(sizeof(double)*n);
	val = 1;
	c = 0;
	for(c = 0; c < n; c++)
	{
		b[c] = val;
		val++;
	}

	
	/*
	// example of small matrix
	int n = 8;
	// column-major order	
	double A[] = {1,0,0,0,0,0,0,0,
					1,-2,0,0,0,0,0,0,
					-1,-3,2,0,0,0,0,0,
					4,1,-3,2,0,0,0,0,
					1,1,1,-4,-4,0,0,0,
					1,-2,5,1,1,5,0,0,
					-1,-3,2,2,2,1,9,0,
					4,1,-3,2,2,2,2,2};

	double b[] = {8,5,0,4,8,5,0,4};
	*/
	double* x = (double*)malloc(sizeof(double)*n);
	
	int d = n/size;
	int first = rank * d;
	int last = (rank + 1) * d - 1;
	int count = d;
	
	worker_struct worker;
	worker.rank = rank;
	worker.size = size;
	worker.first_col = first;
	worker.last_col = last;
	worker.count = count;
	worker.c = (double*)malloc(sizeof(double)*n*d);

	
	struct timeval t1, t2;
	double delta;
	
	if(rank == 0)
		gettimeofday(&t1, NULL);

	/* Split the matrix and start computations */
	MPI_Scatter(A, d*n, MPI_DOUBLE,
				worker.c, d*n, MPI_DOUBLE,
				0, MPI_COMM_WORLD);				
	
	backward_substitution(worker, b, x, n);

	if(rank == 0)
	{
		gettimeofday(&t2, NULL);
		delta = (t2.tv_sec - t1.tv_sec) * 1000.0;
		delta += (t2.tv_usec - t1.tv_usec) / 1000.0;
		printf("Algorithm of %dx%d matrix with %d processors finished after %lf miliseconds = %lf seconds \n",n,n,size, delta, delta/1000.0);
	}

	// print results
	if(rank == 0)
	{
		// sleep just for print
		int i = 0;
		for(i = 0; i < n; i++)
		{
			//printf("x[%d] = %lf \n", i, x[i]);
		}
	}
	
	MPI_Finalize();
	return 0;
}
