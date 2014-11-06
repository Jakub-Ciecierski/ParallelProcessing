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

void *participant_runner(void* ptr);
void *waiter_snack_runner(void* ptr);
void *waiter_wine_runner(void* ptr);
void *table_runner(void* ptr);


const int SNACK_MAX = 5;
const int WINE_MAX = 4;
const int CONSUMPTION_COUNT = 5;

int PARTICIPANT_COUNT;

// when to shut down the table checker
volatile int table_continue = 1;

int debug = 0;

typedef struct 
{	
	int id;
	int count;

	pthread_mutex_t mutex;
}snack_packet;

typedef struct
{
	int id;
	int count;

	pthread_mutex_t mutex;
}wine_packet;
 
 /* 
 * The consumption set for participants
 * Contains information about quantity of snacks and wine
 * and their mutexes.
 */
typedef struct
{
	int id;
	int prev;
	int next; // TODO can replace those with pointers to those packets

	snack_packet* snack;
	wine_packet* wine;
	
	pthread_mutex_t mutex;
	pthread_cond_t condition_var;
}participant_packet;

typedef struct
{
	// id of the current thread
	int id;
	int participant_count;

	participant_packet** dinners;
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

	snack_packet* snack[count/2];
	wine_packet* wine[count/2];
	participant_packet* dinners[count];

	for(i = 0; i < count/2; i++)
	{
		pthread_mutex_t mutex_snack = PTHREAD_MUTEX_INITIALIZER;
		pthread_mutex_t mutex_wine = PTHREAD_MUTEX_INITIALIZER;
		
		// init the memory
		snack[i] = malloc(sizeof(snack_packet));
		wine[i] = malloc(sizeof(wine_packet));
		
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

	// prepare dinner for participants
	for(i = 0; i < count; i++)
	{
		participant_packet* dinner;
		dinners[i] = malloc(sizeof(participant_packet));
		
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
		int prev = (i - 1)%count;
		int next = (i + 1)%count;
		if (prev < 0 )
		{
			prev = count -1;
		}
		dinners[i]->prev = prev;
		dinners[i]->next = next;

		pthread_mutex_t mutex_dinner = PTHREAD_MUTEX_INITIALIZER;
		pthread_cond_t  condition_var   = PTHREAD_COND_INITIALIZER;

		dinners[i]->mutex = mutex_dinner;
		dinners[i]->condition_var = condition_var;
	}

	// create table thread
	pthread_t table_thread;	
	{
		table_packet* table = malloc(sizeof(table_packet));
		table->dinners = dinners;
		table->participant_count = count;

		pthread_create(&table_thread,NULL,&table_runner,(void*)table);
	}

	// create participant threads
	pthread_t participant_threads[count];
	for(i = 0;i < count;i++)
	{
		table_packet* table = malloc(sizeof(table_packet));
		table->dinners = dinners;
		table->id = i;
		table->participant_count = count;

		pthread_t thread;
		pthread_create(&thread,NULL,&participant_runner,(void*)table);
		participant_threads[i] = thread;
		
		// TODO, threads use resources that have not been
		// created yet in this loop,
		sleep(3);
	}

	// create waiter threads
	pthread_t waiter_snack_thread;
	pthread_t waiter_wine_thread;
	{
		table_packet* table = malloc(sizeof(table_packet));
		table->dinners = dinners;
		table->participant_count = count;

		pthread_create(&waiter_snack_thread,NULL,&waiter_snack_runner,(void*)table);
		pthread_create(&waiter_wine_thread,NULL,&waiter_wine_runner,(void*)table);
	}
	// wait for all to finish - VERY IMPORTANT
	for(i = 0;i < count;i++)
	{
		pthread_join(participant_threads[i],NULL);
	}
	pthread_join(table_thread,NULL);
	pthread_join(waiter_snack_thread,NULL);
	pthread_join(waiter_wine_thread,NULL);
}

void *participant_runner(void* ptr)
{
	// receive the data
	table_packet* table = ptr;
	participant_packet** dinners = table->dinners;
	participant_packet* dinner = dinners[table->id];
	
	// take prev and next neighbor
	participant_packet* prev_dinner = dinners[dinner->prev];
	participant_packet* next_dinner = dinners[dinner->next];
	
	if(debug)
	{
		printf("******************************\n");
		printf("P:#%d created with adr:%d\n",dinner->id,dinner);
		printf("Prev P:#%d with adr:%d \n",dinner->prev,prev_dinner);
		printf("Next P:#%d with adr:%d \n",dinner->next,next_dinner);
		printf("snack:#%d \n",dinner->snack->id);
		printf("wine:#%d \n",dinner->wine->id);
		printf("******************************\n\n");
	}

	int i;
	for(i = 0;i < CONSUMPTION_COUNT; i++)
	{
		pthread_mutex_lock(&(dinner->mutex));
		pthread_cond_wait(&(dinner->condition_var), &(dinner->mutex) );

		// lock his prev and next neighbor
		pthread_mutex_lock(&(prev_dinner->mutex));
		pthread_mutex_lock(&(next_dinner->mutex));
		
		// start of critical section
		printf("******************************\n");
		printf("P:#%d Locked P:#%d and P:#%d \n",dinner->id,prev_dinner->id,next_dinner->id);
	
		// consume
		dinner->snack->count--;
		dinner->wine->count--;

		printf("Currect snack:#%d count: %d\n",dinner->snack->id,dinner->snack->count);
		printf("Currect wine:#%d count: %d\n",dinner->wine->id,dinner->wine->count);
		printf("P: #%d leaving the table \n", dinner->id);
		printf("******************************\n\n");
		// end of critical section

		pthread_mutex_unlock(&(next_dinner->mutex));
		pthread_mutex_unlock(&(prev_dinner->mutex));
		pthread_mutex_unlock(&(dinner->mutex));
		sleep(5);
	}
}

void *table_runner(void* ptr)
{
	// receive the data
	table_packet* table = ptr;
	participant_packet** dinners = table->dinners;

	int i = 0;
	while(table_continue)
	{
		for(i = 0;i < table->participant_count;i++)
		{
			participant_packet* dinner = dinners[i];
			if(dinner->snack->count > 0 && dinner->wine->count > 0)
			{
				pthread_cond_signal(&(dinner->condition_var));
			}
		}
	}
}

void *waiter_snack_runner(void* ptr)
{
	// receive the data
	table_packet* table = ptr;
	participant_packet** dinners = table->dinners;
	
	// iterate through all participants
	// and look for non-full bowls of snacks
	while(table_continue)
	{
		printf("************************** \n");
		printf("Snack Waiter \n");
		printf("SW: Locked everyone \n");
		printf("SW: Refilling all bowls of snacks \n");
		int i = 0;
		for(i = 0;i < table->participant_count;i++)
		{
			participant_packet* dinner = dinners[i];
			pthread_mutex_lock(&(dinner->mutex));
		}
		
		for(i = 0;i < table->participant_count;i++)
		{
			participant_packet* dinner = dinners[i];
			dinner->snack->count = SNACK_MAX;
		}

		for(i = 0;i < table->participant_count;i++)
		{
			participant_packet* dinner = dinners[i];
			pthread_mutex_unlock(&(dinner->mutex));
		}
		printf("************************** \n\n");
		sleep(20);
	}
}

void *waiter_wine_runner(void* ptr)
{
	// receive the data
	table_packet* table = ptr;
	participant_packet** dinners = table->dinners;
	
	// iterate through all participants
	// and look for non-full bowls of snacks
	while(table_continue)
	{
		printf("************************** \n");
		printf("Wine Waiter \n");
		int i = 0;
		for(i = 0;i < table->participant_count;i++)
		{
			participant_packet* dinner = dinners[i];
			if(dinner->wine->count == 0)
			{
				// to access specific resource
				// we have to lock this participant
				// and his immediate neighbor that is using the same resource
				participant_packet* prev_dinner = dinners[dinner->prev];
				participant_packet* next_dinner = dinners[dinner->next];
				printf("WW: refilling wine:#%d \n",dinner->wine->id);
				if(dinner->wine->id == prev_dinner->wine->id)
				{
					pthread_mutex_lock(&(dinner->mutex));
					pthread_mutex_lock(&(prev_dinner->mutex));

					printf("WW: Locked P:#%d and P:#%d \n",dinner->id,prev_dinner->id);
					dinner->wine->count = WINE_MAX;				

					pthread_mutex_unlock(&(prev_dinner->mutex));
					pthread_mutex_unlock(&(dinner->mutex));
				}
				else
				{
					pthread_mutex_lock(&(dinner->mutex));
					pthread_mutex_lock(&(next_dinner->mutex));

					printf("WW: Locked P:#%d and P:#%d \n",dinner->id,next_dinner->id);
					dinner->wine->count = WINE_MAX;

					pthread_mutex_unlock(&(next_dinner->mutex));
					pthread_mutex_unlock(&(dinner->mutex));
				}
			}
		}
		printf("************************** \n\n");
		sleep(10);
	}
}