#include <stdio.h>
#include <string.h>
#include <pthread.h> // POSIX
#include <unistd.h> // sleep

/*
 * Compile with:
 * cc -pthread main.c -o main
 */
 
/*
 * Task description
 * 
 * 1) Participants are seated in round table.
 * Number of participants is n >= 4, where n is even
 * 
 * 2) Bottles of wine, and bowls of snacks
 * are placed between the participants.
 * Each paraticipant can only use the resources
 * that are next to him
 * 
 * 3) Participans consume at random time intervals.
 * He needs a bowl of snacks and a bottle of wine,
 * at the same time.
 * 
 * 4) Each bowl or bottle can be used by at most
 * one person at the time.
 * 
 * 5) Each bown contains up to 5 snacks and 
 * each bottle holds 4 glasses of wine.
 * 
 * 7) With some probability, each person raises a toast.
 * During this time noone will start consumption.
 * 
 * 8) Two waiters
 * Waiter 1, replaces empty bottles with full ones
 * Waiter 2, fills all the bowls
 * 
 * 9) After 10 successful consuimptions,
 * the participant goes to sleep - frees the seat
 */

int debug = 0;
void *participant(void* ptr);
void *waiter(void* ptr);
void *table(void* ptr);

//pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

const int SNACK_MAX = 5;
const int WINE_MAX = 4;
const int CONSUMPTION_COUNT = 5;
int PARTICIPANT_COUNT;

typedef struct 
{	
	int id;
	int count;

	pthread_mutex_t mutex;
}snack_state;

typedef struct
{
	int id;
	int count;

	pthread_mutex_t mutex;
}wine_state;
 
 /* The consumption set for participants
 * Contains information about quantity of snacks and wine
 * and their mutexes.
 */
typedef struct
{
	int id;
	int left;
	int right;

	snack_state* snack;
	wine_state* wine;
	
	pthread_mutex_t mutex;
}consumption_set;

typedef struct
{
	// id of the current thread
	int id;
		
	consumption_set** dinners;
}table_packet;

void main(int argc, char** argv)
{
	// Read input and handle errors
	int i = 0;
	int count;
	sscanf(argv[1],"%d",&count);
	if(count%2 != 0 || count < 4)
	{
		printf("incorrect input \n");
		return;
	}

	if(argc > 2)
	{
		debug = 1;
	}
	PARTICIPANT_COUNT = count;

	// init variables
	snack_state* snack[count/2];
	wine_state* wine[count/2];

	for(i = 0; i < count/2; i++)
	{
		pthread_mutex_t mutex_snack = PTHREAD_MUTEX_INITIALIZER;
		pthread_mutex_t mutex_wine = PTHREAD_MUTEX_INITIALIZER;
		
		// init the memory
		snack[i] = malloc(sizeof(snack_state));
		wine[i] = malloc(sizeof(wine_state));
		
		// init the mutexes
		snack[i]->mutex = mutex_snack;
		wine[i]->mutex = mutex_wine;
		
		// init the count
		int snack_count = SNACK_MAX;
		int wine_count = WINE_MAX;
		snack[i]->count = snack_count;
		wine[i]->count = wine_count;
		
		// init id for debug purposes 
		snack[i]->id = i;
		wine[i]->id = i;
	}

	consumption_set* dinners[count];

	// prepare dinner for participants
	for(i = 0; i < count; i++)
	{
		consumption_set* dinner;
		dinners[i] = malloc(sizeof(consumption_set));
		
		// create dinner for each participant
		// according to the set rules
		
		// if participant has is even index
		if(i%2 == 0)
		{
			dinners[i]->snack = snack[(i%count)/2];
			dinners[i]->wine = wine[(i%count)/2];
		}
		// odd indexed participant
		else
		{
			dinners[i]->snack = snack[((i+1)%count)/2];
			dinners[i]->wine = wine[(i-1)/2];
		}
		dinners[i]->id = i;
		int left = (i - 1)%PARTICIPANT_COUNT;
		int right = (i + 1)%PARTICIPANT_COUNT;
		if (left < 0 )
		{
			left = PARTICIPANT_COUNT -1;
		}
		dinners[i]->left = left;
		dinners[i]->right = right;

		pthread_mutex_t mutex_dinner = PTHREAD_MUTEX_INITIALIZER;
		dinners[i]->mutex = mutex_dinner;
	}
	
	// create threads
	pthread_t threads[count];
	for(i = 0;i < count;i++)
	{
		table_packet* table = malloc(sizeof(table_packet));
		table->dinners = dinners;
		table->id = i;
		pthread_t thread;
		pthread_create(&thread,NULL,&participant,(void*)table);
		threads[i] = thread;
		
		// TODO, threads use resources that have not been
		// created yet in this loop,
		sleep(3);
	}

	// wait for all to finish - VERY IMPORTANT
	for(i = 0;i < count;i++)
	{
		pthread_join(threads[i],NULL);
	}
}

void *participant(void* ptr)
{
	// receive the data
	table_packet* table = ptr;
	consumption_set** dinners = table->dinners;
	consumption_set* dinner = dinners[table->id];
	
	// take left and right neighbor
	consumption_set* left_dinner = dinners[dinner->left];
	consumption_set* right_dinner = dinners[dinner->right];
	
	if(debug)
	{
		printf("******************************\n");
		printf("P:#%d created with adr:%d\n",dinner->id,dinner);
		printf("Left P:#%d with adr:%d \n",dinner->left,left_dinner);
		printf("Right P:#%d with adr:%d \n",dinner->right,right_dinner);
		printf("snack:#%d \n",dinner->snack->id);
		printf("wine:#%d \n",dinner->wine->id);
		printf("******************************\n\n");
	}

	int i;
	for(i = 0;i < CONSUMPTION_COUNT; i++)
	{
		pthread_mutex_lock(&(dinner->mutex));
		// lock his left and right neighbor
		pthread_mutex_lock(&(left_dinner->mutex));
		pthread_mutex_lock(&(right_dinner->mutex));
		
		printf("******************************\n");
		printf("P:#%d Locked P:#%d and P:#%d \n",dinner->id,left_dinner->id,right_dinner->id);
	
		// consume
		//dinner->snack->count--;
		//dinner->wine->count--;

		printf("P: #%d leaving the table \n", dinner->id);
		printf("******************************\n\n");

		// end of critical section
		pthread_mutex_unlock(&(dinner->mutex));
		pthread_mutex_unlock(&(left_dinner->mutex));
		pthread_mutex_unlock(&(right_dinner->mutex));
		sleep(5);
	}
}

void *table(void* ptr)
{
	// receive the data
	table_packet* table = ptr;
	consumption_set** dinners = table->dinners;

	int i = 0;
	for(i = 0;i < PARTICIPANT_COUNT;i++)
	{
		consumption_set* dinner = dinners[i];
	}
}
