#define _GNU_SOURCE 
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "mpi.h"

#define LEN 100

int main(int argc, char* argv[])
{
int rank, size, myresult, result;


/* Initialize the environment */
MPI_Init( &argc, &argv );
MPI_Comm_rank( MPI_COMM_WORLD, &rank );
MPI_Comm_size( MPI_COMM_WORLD, &size );

myresult = rank;
result = 0;

printf("My name is %d and my result is %d.\n", rank, myresult);


usleep(10000);

if (rank==0)
{
	printf("\n  Reduction.\n\n");
}

if (rank==1)
{
	usleep(10000000);
}
MPI_Reduce (&myresult,&result,1,MPI_INT,MPI_SUM,0,MPI_COMM_WORLD) ;
//MPI_Allreduce (&myresult,&result,1,MPI_INT,MPI_MAX,MPI_COMM_WORLD);


printf("My name is %d and total result is %d.\n", rank, result);


MPI_Finalize();
return 0;
}



