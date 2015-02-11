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

//int matrix[4][4] = { {1, 2, 3 ,4}, {5, 6, 7, 8}, {9,10,11,12}, {13, 14, 15, 16}};
//int matrix[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
int matrix[12] = {1,2,3,4,5,6,7,8,9,10,11,12};
//int matrix[10] = {1,2,3,4,5,6,7,8,9,10};

int row[3];
char buf[LEN];
int i;

FILE* f;
/* Initialize the environment */
MPI_Init( &argc, &argv );
MPI_Comm_rank( MPI_COMM_WORLD, &rank );
MPI_Comm_size( MPI_COMM_WORLD, &size );

myresult = rank;
result = 0;

MPI_Scatter(matrix,3,MPI_INT,row,3,MPI_INT,0,MPI_COMM_WORLD);


printf("My name is %d and my row is [%d %d %d]. Working on it.\n", rank, row[0], row[1], row[2]);

for (i=0; i<3; ++i)
	row[i]*=rank;
usleep(10000);

if (rank==0)
	usleep(2000000);

MPI_Gather(&row,3,MPI_INT,&matrix,3,MPI_INT,0,MPI_COMM_WORLD);
//MPI_Allgather(row,3,MPI_INT,matrix,3,MPI_INT,MPI_COMM_WORLD);

sprintf(buf, "My name is %d and the matrix is [", rank);
for (i=0; i< 12; ++i)
	sprintf(buf, "%s %d", buf, matrix[i]);
sprintf(buf, "%s ]\n", buf);


printf(buf);

MPI_Finalize();
return 0;
}



