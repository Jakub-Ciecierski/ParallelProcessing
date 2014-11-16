#include <stdio.h>
#include <string.h>
#include <pthread.h> // POSIX
#include <unistd.h> // sleep
#include <stdlib.h> // random
#include <time.h> // time for seed

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

typedef enum {EATING, HUNGRY, THINKING, ABOUT_TO_EAT} state_t;

const int SNACK_MAX = 5;
const int WINE_MAX = 4;
const int CONSUMPTION_COUNT = 10;

// used to generate random numbers
unsigned int seed;

int PARTICIPANT_COUNT;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// when to shut down the table checker
volatile int table_continue = 1;

int debug = 0;

/*
 * Snack packet, holding its count and virtual consumption
 */
typedef struct 
{	
	int id;
	int count;
	int virtual_consumption;
}snack_packet;

/*
 * Wine packet, holding its count and virtual consumption
 */
typedef struct
{
	int id;
	int count;
	int virtual_consumption;
}wine_packet;
 
 /* 
 * The consumption set for participants
 * Contains information about their snack and wine
 * and their condition variables.
 */
 typedef struct participant_packet participant_packet; 
 
struct participant_packet
{
	int id;
	participant_packet* prev;
	participant_packet* next;

	snack_packet* snack;
	wine_packet* wine;
	
	state_t state;

	pthread_cond_t condition_var;
};

typedef struct
{
	// id of the current thread
	int id;
	int participant_count;

	participant_packet** dinners;
	
	snack_packet** snacks;
	wine_packet** wines;
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
	seed = time(NULL);
	snack_packet* snack[count/2];
	wine_packet* wine[count/2];
	participant_packet* dinners[count];
	

	for(i = 0; i < count/2; i++)
	{
		// init the memory
		snack[i] = malloc(sizeof(snack_packet));
		wine[i] = malloc(sizeof(wine_packet));

		int virtual_consumption_snack = 0;
		int virtual_consumption_wine = 0;
		// init virtual consumption
		snack[i]->virtual_consumption = virtual_consumption_snack;
		wine[i]->virtual_consumption = virtual_consumption_wine;
		
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
		dinners[i]->state = THINKING;

		pthread_cond_t  condition_var   = PTHREAD_COND_INITIALIZER;

		dinners[i]->condition_var = condition_var;
	}
	
	// set up previous and next pointers
	for(i = 0; i < count; i++)
	{
		int prev = (i - 1)%count;
		int next = (i + 1)%count;
		if (prev < 0 )
		{
			prev = count -1;
		}
		dinners[i]->prev = dinners[prev];
		dinners[i]->next = dinners[next];
	}

	// create participant thread1s
	pthread_t participant_threads[count];
	for(i = 0;i < count;i++)
	{
		table_packet* table = malloc(sizeof(table_packet));
		table->dinners = dinners;
		table->id = i;
		table->participant_count = count;
		table->snacks = snack;
		table->wines = wine;

		pthread_t thread;
		pthread_create(&thread,NULL,&participant_runner,(void*)table);
		participant_threads[i] = thread;
	}

	// create waiter threads
	pthread_t waiter_snack_thread;
	pthread_t waiter_wine_thread;
	table_packet* table = malloc(sizeof(table_packet));
	table->dinners = dinners;
	table->participant_count = count;
	table->snacks = snack;
	table->wines = wine;
	
	
	pthread_create(&waiter_snack_thread,NULL,&waiter_snack_runner,(void*)table);
	pthread_create(&waiter_wine_thread,NULL,&waiter_wine_runner,(void*)table);

	// wait for all to finish
	for(i = 0;i < count;i++)
	{
		pthread_join(participant_threads[i],NULL);
	}
	// stop the waiters
	table_continue = 0;

	pthread_join(waiter_snack_thread,NULL);
	pthread_join(waiter_wine_thread,NULL);
}

void *participant_runner(void* ptr)
{
	// receive the data
	table_packet* table = ptr;
	participant_packet** dinners = table->dinners;
	participant_packet* dinner = dinners[table->id];

	if(debug)
	{
		printf("******************************\n");
		printf("P:#%d created with dinners adr:%d\n",dinner->id,dinners);
		printf("P:#%d created with adr:%d\n",dinner->id,dinner);
		printf("P:#%d created with mutex adr:%d\n",dinner->id,&(mutex));
		printf("Prev P:#%d with adr:%d \n",dinner->prev->id,dinner->prev);
		printf("Next P:#%d with adr:%d \n",dinner->next->id,dinner->next);
		printf("snack:#%d \n",dinner->snack->id);
		printf("wine:#%d \n",dinner->wine->id);
		printf("******************************\n\n");
	}

	int i;
	for(i = 0;i < CONSUMPTION_COUNT; i++)
	{
		// THINK
		int sleep_time = rand_r(&seed) % 21;
		printf("P: #%d going to THINK for: %d seconds \n", dinner->id, sleep_time);
		sleep(sleep_time);
		// END OF THINK

		// GET RESOURCES
		pthread_mutex_lock(&(mutex));

		// set state to hungry, he is intrested in resources
		dinner->state = HUNGRY;

		printf("P:#%d is HUNGRY \n",dinner->id);

		// if one of his neighbors are eating
		// or his resources are not avaible, wait in condition variable
		if(!(dinner->prev->state != EATING
			&& dinner->next->state != EATING
			&& dinner->snack->count - dinner->snack->virtual_consumption > 0
			&& dinner->wine->count - dinner->wine->virtual_consumption > 0 ))
		{	
			printf("P:#%d is waiting for his turn \n",dinner->id);

			pthread_cond_wait(&(dinner->condition_var), &(mutex));			

			printf("P:#%d got released, about to eat \n",dinner->id);

			// when signaled, fix the virtual consumption value
			dinner->snack->virtual_consumption--;
			dinner->wine->virtual_consumption--;
		}

		// now he is ready to eat
		dinner->state = EATING;
		pthread_mutex_unlock(&(mutex));
		// END OF GET RESOURCES

		printf("******************************\n");

		// EATING
		dinner->snack->count--;
		dinner->wine->count--;
		int j = 0;
		int eating_time = 5;
		for(j = 0;j < eating_time;j++)
		{
			// simulate eating
			printf("P:#%d eating...\n",dinner->id);
			sleep(1);
		}
		printf("******************************\n\n");
		// END OF EATING

		// PUT RESOURCES
		pthread_mutex_lock(&(mutex));
		dinner->state = THINKING;

		// wake up previous if possible
		if(dinner->prev->state == HUNGRY)
		{
			// check if previous of his previous is not eating
			if(dinner->prev->prev->state != EATING
				&& dinner->prev->snack->count - dinner->prev->snack->virtual_consumption > 0
				&& dinner->prev->wine->count - dinner->prev->wine->virtual_consumption > 0)
			{
					dinner->prev->snack->virtual_consumption++;
					dinner->prev->wine->virtual_consumption++;

					printf("P:#%d signaled P:#%d \n",dinner->id, dinner->prev->id);
					
					// to prevent that somebody else signals him twice
					// change the state to ABOUT_TO_EAT
					dinner->prev->state = ABOUT_TO_EAT;
					pthread_cond_signal(&(dinner->prev->condition_var));
			}
		}
		// wake up next if possible
		if(dinner->next->state == HUNGRY)
		{
			// check if next of his next is not eating
			if(dinner->next->next->state != EATING
				&& dinner->next->snack->count - dinner->next->snack->virtual_consumption > 0
				&& dinner->next->wine->count - dinner->next->wine->virtual_consumption> 0)
			{
					dinner->next->snack->virtual_consumption++;
					dinner->next->wine->virtual_consumption++;
					printf("P:#%d signaled P:#%d \n",dinner->id, dinner->next->id);
					
					// to prevent that somebody else signals him twice
					// change the state to ABOUT_TO_EAT
					dinner->next->state = ABOUT_TO_EAT;
					pthread_cond_signal(&(dinner->next->condition_var));
			}
		}
		// TOAST

		// to toast, simply go to sleep inside the global mutex
		// no other participant will start eating
		// THESE ALREADY EATING WILL FINISH THEIR CONSUMPTION BEFORE GOING TO LISTEN
		int toast = rand_r(&seed) % 5;
		if(toast == 0)
		{
			int toast_time = 15;
			printf("P:#%d starting toast, nobody will start consumption for: %d seconds \n\n",dinner->id,toast_time);
			int k = toast_time;
			for(k = toast_time; k > 0; k--)
			{
				printf("Toast: %d more seconds... \n",k);
				sleep(1);
			}
		}
		// END OF TOAST

		pthread_mutex_unlock(&(mutex));
		// END OF PUT RESOURCES
	}
	printf("P: #%d leaving the table forever\n", dinner->id);
}


void *waiter_snack_runner(void* ptr)
{
	// receive the data
	table_packet* table = ptr;
	participant_packet** dinners = table->dinners;
	
	if(debug)
	{
		printf("******************************\n");
		printf("Snack Waiter created with mutex: %d\n",&(mutex));
		printf("Snack Waiter created with dinners adr:%d\n",table->dinners);
		int j = 0;
		for(j = 0; j < table->participant_count; j++)
		{
			printf("P:#%d with dinner adr:%d\n",dinners[j]->id,dinners[j]);
		}
		printf("******************************\n\n");
	}
	
	while(table_continue)
	{
		pthread_mutex_lock(&(mutex));

		printf("************************** \n");
		printf("Snack Waiter \n");
		printf("SW: Locked everyone \n");
		printf("SW: Refilling all bowls of snacks \n");

		int i = 0;
		for(i = 0;i < table->participant_count;i++)
		{
			participant_packet* dinner = dinners[i];
			dinner->snack->count = SNACK_MAX; //TODO synchronize

			// signal hungry participants which are eligible for eating
			if(dinner->state == HUNGRY)
			{
				if(dinner->prev->state != EATING
					&& dinner->next->state != EATING
					&& dinner->snack->count - dinner->snack->virtual_consumption > 0
					&& dinner->wine->count - dinner->wine->virtual_consumption > 0 )
				{
					dinner->snack->virtual_consumption++;
					dinner->wine->virtual_consumption++;
					printf("SW: signaled P:#%d \n",dinner->id);
					
					// to prevent that somebody else signals him twice
					// change the state to ABOUT_TO_EAT
					dinner->state = ABOUT_TO_EAT;
					pthread_cond_signal(&(dinner->condition_var));
				}
			}
		}
		
		// printf current state
		int j = 0;
		for(j = 0; j < (table->participant_count)/2; j++)
		{
			snack_packet* snack = (table->snacks)[j];
			wine_packet* wine = (table->wines)[j];
			printf("Currect snack:#%d count: %d\n",snack->id, snack->count);
			printf("Currect wine:#%d count: %d\n",wine->id, wine->count);
			printf("Currect wine:#%d virtual_consumption: %d\n",snack->id,snack->virtual_consumption);
			printf("Currect wine:#%d virtual_consumption: %d\n\n",wine->id,wine->virtual_consumption);
		}
		printf("************************** \n\n");
		pthread_mutex_unlock(&(mutex));
		
		int waiter_sleep = 35;
		sleep(waiter_sleep);
	}
}

void *waiter_wine_runner(void* ptr)
{
	// receive the data
	table_packet* table = ptr;
	participant_packet** dinners = table->dinners;
	
	if(debug)
	{
		printf("******************************\n");
		printf("Wine Waiter created with mutex: %d\n",&(mutex));
		printf("Wine Waiter created with dinners adr:%d\n",table->dinners);
		printf("******************************\n\n");
	}
	
	while(table_continue)
	{
		pthread_mutex_lock(&(mutex));
		printf("************************** \n");
		printf("Wine Waiter \n");
		int i = 0;
		for(i = 0;i < table->participant_count;i++)
		{
			participant_packet* dinner = dinners[i];
			if(dinner->wine->count == 0)
			{
				dinner->wine->count = WINE_MAX;
				printf("WW: refilling wine:#%d \n",dinner->wine->id);
				
				// signal hungry participants which are eligible for eating
				if(dinner->state == HUNGRY)
				{
					if (dinner->prev->state != EATING
						&& dinner->next->state != EATING
						&& dinner->snack->count - dinner->snack->virtual_consumption > 0
						&& dinner->wine->count - dinner->wine->virtual_consumption > 0 )
					{
						dinner->snack->virtual_consumption++;
						dinner->wine->virtual_consumption++;

						printf("WW: signaled P:#%d \n",dinner->id);

						// to prevent that somebody else signals him twice
						// change the state to ABOUT_TO_EAT
						dinner->state = ABOUT_TO_EAT;
						pthread_cond_signal(&(dinner->condition_var));
					}
				}
			}
		}

		// print current state
		int j = 0;
		for(j = 0; j < (table->participant_count)/2; j++)
		{
			snack_packet* snack = (table->snacks)[j];
			wine_packet* wine = (table->wines)[j];
			printf("Currect snack:#%d count: %d\n",snack->id, snack->count);
			printf("Currect wine:#%d count: %d\n",wine->id, wine->count);
			printf("Currect wine:#%d virtual_consumption: %d\n",snack->id, snack->virtual_consumption);
			printf("Currect wine:#%d virtual_consumption: %d\n\n",wine->id, wine->virtual_consumption);
		}
		printf("************************** \n\n");
		pthread_mutex_unlock(&(mutex));
		
		int waiter_sleep = 25;
		sleep(waiter_sleep);
	}
}
