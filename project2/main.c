#include <stdio.h>
#include <string.h>
#include <pthread.h>

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

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int SNACK_MAX = 5;
int WINE_MAX = 4;

/* The consumption set for participants
 * Contains information about quantity of snacks and wine
 * and their mutexes.
 */
 
 typedef struct 
{	
	int count;
	int id;

	pthread_mutex_t mutex;
}snack_state;

typedef struct
{
	int count;
	int id;

	pthread_mutex_t mutex;
}wine_state;
 
typedef struct
{
	snack_state* snack;
	wine_state* wine;
	int id;
}consumption_set;

void *participant(void* ptr);

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

	if(debug)printf("initialize mutexes start\n");
	// initialize mutexes

	snack_state* snack[count/2];
	wine_state* wine[count/2];

	for(i = 0; i < count/2; i++)
	{
		pthread_mutex_t mutex_snack = PTHREAD_MUTEX_INITIALIZER;
		pthread_mutex_t mutex_wine = PTHREAD_MUTEX_INITIALIZER;
		
		// init the memory
		snack[i] = (snack_state*)malloc(sizeof(snack_state));
		wine[i] = (wine_state*)malloc(sizeof(wine_state));
		
		// init the mutexes
		snack[i]->mutex = mutex_snack;
		wine[i]->mutex = mutex_wine;
		
		// init the count
		int snack_count = SNACK_MAX;
		int wine_count = WINE_MAX;
		snack[i]->count = snack_count;
		wine[i]->count = wine_count;
		
		// debug purpose, init id
		snack[i]->id = i;
		wine[i]->id = i;
	}
	if(debug)printf("initialize mutexes finish\n");
	
	if(debug)printf("create threads start\n");
	// create threads
	pthread_t threads[count];
	for(i = 0; i < count; i++)
	{
		consumption_set* dinner;
		dinner = (consumption_set*)malloc(sizeof(consumption_set));
		
		// create dinner for each participant
		// according to the set rules
		
		// if participant has is even index
		if(debug)printf("create threads 1\n");
		if(i%2 == 0)
		{
			dinner->snack = snack[(i%count)/2];
			dinner->wine = wine[(i%count)/2];
		}
		// odd indexed participant
		else
		{
			dinner->snack = snack[((i+1)%count)/2];
			dinner->wine = wine[(i-1)/2];
		}
		dinner->id = i;

		if(debug)printf("create threads 2\n");
		pthread_t thread;
		pthread_create(&thread,NULL,&participant,(void*)dinner);
		threads[i] = thread;
	}
	if(debug)printf("create threads finish\n");
	
	// wait for all to finish
	for(i = 0;i < count;i++)
	{
		pthread_join(threads[i],NULL);
	}
}

void *participant(void* ptr)
{
	consumption_set* dinner = ptr;
	pthread_mutex_lock(&mutex);
	
	printf("Participant nr: %d \n snack: %d \n wine: %d \n",dinner->id,dinner->snack->id,dinner->wine->id);
	//printf("Current snack #%d count: %d\n",dinner->snack->id,dinner->snack->count);
	if(dinner->snack->count > 0)
	{
		printf("Participant: #%d consuming snack: #%d \n",dinner->id,dinner->snack->id);
		
		dinner->snack->count--;
		
		printf("Snack #%d memory address: %d \n",dinner->snack->id,dinner->snack);
		
	}
	pthread_mutex_unlock(&mutex);
}
