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
 * 5) Each member computes the difference between the number of citizen in each of his cities
 * 	  and the average number of citizens
 * 6) Each member sends the list of his cities with this computed difference to the Boss
 * 7) Boss writes report to the console
 */
 
#define NAME_LEN 10
#define BOSS 0

typedef struct
{
	int citizen_count;
	char name[NAME_LEN];
}city_packet;

int main(int argc, char* argv[])
{
	int rank, size;

	// Initialize the environment
	MPI_Init( &argc, &argv );
	MPI_Comm_rank( MPI_COMM_WORLD, &rank );
	MPI_Comm_size( MPI_COMM_WORLD, &size );

	// create MPI_Datatype
	MPI_Datatype city_type;
	int blocklens[2];
	MPI_Aint indices[2];
	MPI_Datatype old_types[2];

	// get the extent of int
	MPI_Aint extent;
	MPI_Type_extent(MPI_INT,&extent);

	blocklens[0] = 1;
	blocklens[1] = NAME_LEN;

	old_types[0] = MPI_INT;
	old_types[1] = MPI_CHAR;

	indices[0] = 0;
	indices[1] = extent;

	MPI_Type_create_struct(
	  2, blocklens, indices, old_types, &city_type
	);
	
	//MPI_Type_struct( 2, blocklens, indices, old_types, &city_type );
    MPI_Type_commit( &city_type );

	char* file = argv[1];

	// number of members
	int n = size;

	// total number of cities to select
	int cities_count = 0;

	/*
	 * 0) A fast, working but not nice hack to compute number of line,
	 * simply read the file twice
	 */
	if(rank == BOSS)
	{
		printf("\n******** STEP 0 ********\n"); 
		printf("Check number of cities, allocate memory for packages \n"); 

		FILE* f;
		f = fopen(file,"r");

		int len = 0;
		char* line = NULL;

		while(getline(&line, &len, f) != -1)
		{
			cities_count++;
			
			len = 0;
			line = NULL;
		}
		fclose(f);
	}

	MPI_Bcast(&cities_count, 1, MPI_INT,
				BOSS, MPI_COMM_WORLD);
	
	// numbers of cities for every member to work on
	int k = cities_count / n;
	// all selected cities
	city_packet all_cities[cities_count];
	// k cities
	city_packet k_cities[k];

	if(rank == BOSS)
	{
		printf("[n]: %d members, each [k]: %d cities, in total: %d \n", n, k, cities_count); 
	}
	
	// for output readability
	sleep(1);
	
	/*
	 * 1) Boss reads N*K cities from file. 
	 */
	if(rank == BOSS)
	{
		printf("\n******** STEP 1 ********\n"); 
		printf("The boss reads the file \n"); 
		FILE* f;

		f = fopen(file,"r");

		int len = 0;
		char* line = NULL;

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

			// get the citizen count
			int citizen_count = 0;
			char name[10];
	
			// [^characters] Negated scanset	
			// Any number of characters none of them specified as characters between the brackets.
			sscanf(line, "%[^,],%d", name, &citizen_count);
			
			all_cities[i].citizen_count = citizen_count;
			strcpy(all_cities[i].name, name);

			printf("City nr#%d: %s : %d \n",i+1,all_cities[i].name,all_cities[i].citizen_count);
		}
		printf("The boss finished reading the file \n");
		fclose(f);
	}

	// for output readability
	sleep(1);
	
	/* 
	 * 2) Boss (who is also a member) sends K cities to each member. 
	 */
	if (rank == BOSS)
	{
		printf("\n******** STEP 2 ********\n"); 
		printf("Boss Scatters %d cities to each member\n",k);
	}

	MPI_Scatter(all_cities, k, city_type,
				k_cities, k, city_type,
				BOSS, MPI_COMM_WORLD);				

	int l = 0;
	for(l = 0; l < k; l++)
	{
		printf("Member #%d received city: %s, %d\n", rank, k_cities[l].name, k_cities[l].citizen_count);
	}
	
	// for output readability
	sleep(1);
	
	/*
	 * 3) Each member sends the Boss total number of citizens in his cities. 
	 */
	if (rank == BOSS)
	{
		printf("\n******** STEP 3,4 ********\n"); 
		printf("Each member computes total number of citizens in his cities \n");
		printf("The boss computes the average number of citizens and sends it to all team members \n");
	}
	
	// compute total count of citizens
	int k_citizen_count = 0;
	int i = 0;
	for(i = 0; i < k; i++)
	{
		k_citizen_count += k_cities[i].citizen_count;
	}

	// for output readability
	sleep(1);
	printf("Member#%d computed %d citizens \n",rank, k_citizen_count);
	
	// Use Reduce to compute sum of all k citizen sums
	int n_citizen_count = 0;
	MPI_Reduce(&k_citizen_count, &n_citizen_count, 1, 
				MPI_INT, MPI_SUM,
				BOSS, MPI_COMM_WORLD);

	int average_count = 0;
	if(rank == BOSS)
	{
		printf("After Reduce MPI_SUM, Boss received sum of all citizens: %d\n", n_citizen_count);
		average_count = n_citizen_count / cities_count;
		printf("The average of all cities: %d\n", average_count);
	}
	
	/* 
	 * 4) The boss computes the average number of citizens and sends it to all team members 
	 */
	MPI_Bcast(&average_count, 1, MPI_INT,
				BOSS, MPI_COMM_WORLD);
	

	// for output readability		
	sleep(1);

	/* 
	 * 5) Each member computes the difference between the number of citizen in each of his cities
  	 * and the average number of citizens
	 * 6) Each member sends the list of his cities with this computed difference to the Boss
	 */
	if (rank == BOSS)
	{
		printf("\n******** STEP 5, 6 ********\n");
		printf("Each member computes the difference between the number of citizen in each of his cities and the average number of citizens\n");
		printf("Each member sends the list of his cities with this computed difference to the Boss \n");
	}

	i = 0;
	// compute the difference
	for(i = 0;i < k; i++)
	{
		k_cities[i].citizen_count = k_cities[i].citizen_count - average_count;
	}
	
	// Boss gathers all the data
	MPI_Gather(&k_cities, k, city_type,
				&all_cities, k, city_type,
				BOSS, MPI_COMM_WORLD);
		
	// for output readability
	sleep(1);

	/*
	 * 7) Boss writes report to the console
	 */
	if(rank == BOSS)
	{
		printf("\n******** STEP 7 ********\n");
		printf("The report \n\n");
		i = 0;
		for(i = 0;i < cities_count; i++)
		{
			printf("%s, %d \n", all_cities[i].name, all_cities[i].citizen_count);
		}
	}
	MPI_Type_free( &city_type );
	MPI_Finalize();
	return 0;
}