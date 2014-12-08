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
int rank, size;
char buf[LEN];
MPI_Status status;

/* Initialize the environment */
MPI_Init( &argc, &argv );
MPI_Comm_rank( MPI_COMM_WORLD, &rank );
MPI_Comm_size( MPI_COMM_WORLD, &size );
srand(rank);

snprintf(buf, LEN, "%d should become a president", rank);
printf("Process %d thinks: %s\n", rank, buf);


if (rank==0)
{
	printf("\n  Process %d uses blackmail. \n\n", rank);
	
	snprintf(buf, LEN, "%d FOR PRESIDENT *wink*",rank);
	usleep(1000000);
}

//printf("Process %d before MPI_Bcast \n", rank);
MPI_Bcast(buf, LEN, MPI_CHAR, 0, MPI_COMM_WORLD);	
//printf("Process %d after MPI_Bcast \n", rank);


printf("Process %d votes for: %s\n", rank, buf);


MPI_Finalize();
return 0;
}



