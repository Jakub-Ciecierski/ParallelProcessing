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


int main(int argc, char* argv[])
{
int rank, size, len;
int time;
char name[MPI_MAX_PROCESSOR_NAME];


/* Initialize the environment */
MPI_Init( &argc, &argv );
MPI_Comm_rank( MPI_COMM_WORLD, &rank );
MPI_Comm_size( MPI_COMM_WORLD, &size );
MPI_Get_processor_name(name, &len);
srand(rank);
printf("Process %d from processor %s starts.\n", rank, name);

time = rand() % 10;
usleep(time * 1000000);

if (rank ==0)
{
	MPI_Barrier(MPI_COMM_WORLD);
}
if (rank == 1)
{
	MPI_Barrier(MPI_COMM_WORLD);
}
if (rank == 2)
{
	MPI_Barrier(MPI_COMM_WORLD);
}

	

printf("Process %d from processor %s finishes.\n", rank, name);
MPI_Finalize();
return 0;
}



