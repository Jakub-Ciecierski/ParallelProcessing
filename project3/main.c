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

/**
 * 0) N members.
 * 1) Boss reads N*K cities from file.
 * 2) Boss (who is also a member) sends K cities to each member.
 * 3) Each member sends the Boss total number of citizens in his cities.
 * 4) The boss computes the average number of citizens and sends it to all team members
 * 5) Each members computes the difference between the number of citizen in each of his cities
 * 	  and the average number of citizens
 * 6) Each member sends the list of his cities with this computed difference to the Boss
 * 7) Boss writes report to the console
 */

#define CITY_LEN 10
#define BOSS 0

typedef struct
{
	char name[CITY_LEN];
	int citizen_count;
}city_packet;

int main(int argc, char* argv[])
{
	int rank, size;

	// Initialize the environment
	MPI_Init( &argc, &argv );
	MPI_Comm_rank( MPI_COMM_WORLD, &rank );
	MPI_Comm_size( MPI_COMM_WORLD, &size );
	
	if( rank == BOSS)
	{
		printf("\n******** STEP 0 ********\n"); 		
	}
	
	/*
	// create MPI_Datatype
	MPI_Datatype city_struct;
	int blocklens[2];
	MPI_Aint indices[2];
	MPI_Datatype old_types[2];
	
	blocklens[0] = 10;
	blocklens[1] = 1;
	
	old_types[0] = MPI_CHAR;
	old_types[1] = MPI_INT;
	
	MPI_Address( &city_packet.name, &indices[0] );
    MPI_Address( &city_packet.citizen_count, &indices[1] );
	
	indices[1] = indices[1] - indices[0];
	indices[0] = 0;
	
	MPI_Type_struct( 2, blocklens, indices, old_types, &city_struct );
    MPI_Type_commit( &city_struct );
	*/

	// numbers of cities for every member to work on
	int k = 3;
	// number of members
	int n = size;
	// total number of cities to select
	int cities_count  = n*k;
	// all selected cities
	city_packet* all_cities[cities_count];
	city_packet* k_cities[k];

	int all_citizens[cities_count];
	int k_citizens[k];
	
	/*
	 * 1) Boss reads N*K cities from file. 
	 */
	if(rank == BOSS)
	{
		printf("\n******** STEP 1 ********\n"); 
		printf("The boss reads the cities.txt file \n"); 
		FILE* f;

		f = fopen("cities.txt","r");

		// read all selected cities
		int i = 0;

		for(i = 0;i < cities_count; i++)
		{
			// get a line from the file			
			int len = 0;
			char* line = NULL;

			int ret = getline(&line, &len, f);
			
			// exit on error - assuming that there are enough cities provided
			if(ret == -1)
			{
				perror("File has not enough cities, shuting down \n");
				exit(0);
			}
			
			all_cities[i] = malloc(sizeof(city_packet));

			// get the citizen count
			int citizen_count = 0;
			char name[10];
	
			sscanf(line, "%[^,],%d", name, &citizen_count);
			
			all_cities[i]->citizen_count = citizen_count;
			strcpy(all_cities[i]->name, name);
			all_citizens[i] = citizen_count;

			printf("City nr#%d: %s : %d \n",i+1,all_cities[i]->name,all_cities[i]->citizen_count);
		}
		printf("The boss finished reading the file \n");
		close(f);
	}
	
	/* 
	 * 2) Boss (who is also a member) sends K cities to each member. 
	 */
	if (rank == BOSS)
	{
		printf("\n******** STEP 2 ********\n"); 
		printf("Boss Scatters %d cities to each member\n",k);
	}

	MPI_Scatter(all_citizens, k, MPI_INT,
				k_citizens, k, MPI_INT,
				BOSS, MPI_COMM_WORLD);
				
	printf("Member #%d received %d cities\n", rank, k);

	/*
	 * 3) Each member sends the Boss total number of citizens in his cities. 
	 */
	
	if (rank == BOSS)
	{
		printf("\n******** STEP 3 ********\n"); 
		printf("Each member computes total number of citizens in his cities then the Boss Gathers the data \n");
	}
	sleep(2);
	
	// compute total count of citizens
	int k_citizen_count = 0;
	int i = 0;
	for(i = 0; i < k; i++)
	{
		k_citizen_count += k_citizens[i];
	}

	printf("Member#%d computed %d citizens \n",rank, k_citizen_count);
	
	// Boss gathers all the total counts
	int member_citizen_counts[n];
	MPI_Gather(k_citizen_count, 1, MPI_INT,
				member_citizen_counts, 1, MPI_INT,
				BOSS, MPI_COMM_WORLD);
	
	if(rank == BOSS)
	{
		printf("The Boss received total counts \n");
		i = 0;
		for(i = 0; i < n; i++)
		{
			printf("#%d: %d\n",i, member_citizen_counts[i]);
		}
	{
	sleep(rank);
	
	/* 
	 * 4) The boss computes the average number of citizens and sends it to all team members 
	 */
	if (rank == BOSS)
	{
		printf("\n******** STEP 4 ********\n");
		printf("Boss computes the average number of citizens and Scatters it to all members\n");
	}
	
	int average_count = 0;
	if( rank == BOSS)
	{
		for(i = 0; i < n; i++)
		{
			average_count += member_citizen_counts[i];
		}
		average_count = average_count/cities_count;
		printf("Boss computes the average: %d", average_count);
	}
	
	MPI_Scatter(&average_count, 1, MPI_INT,
				BOSS, MPI_COMM_WORLD);
	
	printf("I'm #%d and received average: %d", rank, average_count);
	
	/* 
	 * 5) Each members computes the difference between the number of citizen in each of his cities
  	 * and the average number of citizens
	 */
	if (rank == BOSS)
	{
		printf("\n******** STEP 5 ********\n");
		printf("Each members computes the difference between the "
				+ "number of citizen in each of his cities and the average number of citizens\n");
	}
	
	
	//MPI_Type_free( &city_struct );
	MPI_Finalize();
	return 0;
}