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

#define LEN 20

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

snprintf(buf, LEN, " My name is %d", rank);

printf("Process %d sends a message to process %d\n", rank, (rank+1)%size);
usleep(10000);
MPI_Send(buf, LEN, MPI_CHAR, (rank+1)%size, 0, MPI_COMM_WORLD);
printf("Process %d after MPI_Send\n", rank);

//MPI_Recv(buf,LEN,MPI_CHAR,MPI_ANY_SOURCE,0,MPI_COMM_WORLD,&status);

//MPI_Send(buf, LEN, MPI_CHAR, (rank+1)%2, 1, MPI_COMM_WORLD);

//MPI_Recv(buf,LEN,MPI_CHAR,MPI_ANY_SOURCE,1,MPI_COMM_WORLD,&status);


//MPI_Recv(buf,LEN,MPI_CHAR,MPI_ANY_SOURCE,MPI_ANY_TAG,MPI_COMM_WORLD,&status);

printf(" Process %d received a message %s with tag %d from %d \n", rank, buf, status.MPI_TAG, status.MPI_SOURCE);


MPI_Finalize();
return 0;
}



