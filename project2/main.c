#include <stdio.h>
#include <string.h>
#include <pthread.h>	// POSIX
#include <unistd.h> 	// sleep
#include <stdlib.h> 	// random
#include <time.h> 		// time for seed

#define KGRN  "\x1B[32m"
#define RESET "\033[0m"

/*
 * Task description:
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

// thread functions 
void *participant_runner(void* ptr);
void *waiter_snack_runner(void* ptr);
void *waiter_wine_runner(void* ptr);

/*
 * States of Participant
 *
 * EATING - When Participant is actually eating
 * HUNGRY - When Participant is interested in eating,
 * 			in that time he checks if he can eat and
 *			waits if needed.
 * THINKING - When Participant is thinking, idle
 * ABOUT_TO_EAT - When Participant got signaled by other thread,
 * 				indicating that he's able to eat but still waiting
 *				for that thread or more to unlock the mutex.
 */
typedef enum {EATING, HUNGRY, THINKING, ABOUT_TO_EAT} state_t;

const int SNACK_MAX = 5;
const int WINE_MAX = 4;
const int CONSUMPTION_COUNT = 10;

// used to generate random numbers
unsigned int seed;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// when to shut down the table
volatile int table_continue = 1;

/*
 * Snack packet, holding its count and virtual consumption
 */
typedef struct 
{	
	int id;
	int count;
}snack_packet;

/*
 * Wine packet, holding its count and virtual consumption
 */
typedef struct
{
	int id;
	int count;
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

	seed = time(NULL);
	snack_packet* snack[count/2];
	wine_packet* wine[count/2];
	participant_packet* dinners[count];

	for(i = 0; i < count/2; i++)
	{
		// init the memory
		snack[i] = malloc(sizeof(snack_packet));
		wine[i] = malloc(sizeof(wine_packet));
		
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
		
		// if participant has even index
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

		pthread_cond_t  condition_var  = PTHREAD_COND_INITIALIZER;

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

	printf("\n Waiting for waiters to clean up... \n");

	pthread_join(waiter_snack_thread,NULL);
	pthread_join(waiter_wine_thread,NULL);
	
	printf("\n Program finishing... \n");
	return;
}

void *participant_runner(void* ptr)
{
	// receive the data
	table_packet* table = ptr;
	participant_packet** dinners = table->dinners;
	participant_packet* dinner = dinners[table->id];

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

		// If one of his neighbors are eating
		// or his resources are not avaible, wait in condition variable.
		// The order of statements is important.
		// First We check if any neighbor is eating to make sure
		// we don't access resources that are being used.
		if(!((dinner->prev->state != EATING) && (dinner->prev->state != ABOUT_TO_EAT )
			&& (dinner->next->state != EATING) && (dinner->next->state != ABOUT_TO_EAT) 
			&& (dinner->snack->count > 0) && (dinner->wine->count > 0 )))
		{
			printf("P:#%d is waiting for his turn \n",dinner->id);

			pthread_cond_wait(&(dinner->condition_var), &(mutex));			

			printf("P:#%d got released, about to eat \n",dinner->id);
		}

		// now he is ready to eat
		dinner->state = EATING;
		pthread_mutex_unlock(&(mutex));
		// END OF GET RESOURCES

		// EATING

		// Happens outside critical section.
		// Thanks to states we are sure that these resources
		// are used by only one process.
		printf("******************************\n");
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
		// END OF EATING

		// PUT RESOURCES
		pthread_mutex_lock(&(mutex));
		
		printf("P:#%d Finished eating \n",dinner->id);
		printf(KGRN "P:#%d snack count: %d\n" RESET,dinner->id, dinner->snack->count);
		printf(KGRN "P:#%d wine count: %d\n" RESET,dinner->id, dinner->wine->count);
		printf("******************************\n\n");

		dinner->state = THINKING;

		// wake up previous if possible
		if(dinner->prev->state == HUNGRY)
		{
			// check if previous of his previous is not eating
			if((dinner->prev->prev->state != EATING ) && (dinner->prev->prev->state != ABOUT_TO_EAT )
				&& (dinner->prev->snack->count > 0 ) && (dinner->prev->wine->count > 0))
			{
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
			if((dinner->next->next->state != EATING) && (dinner->next->next->state != ABOUT_TO_EAT)
				&& (dinner->next->snack->count > 0) && (dinner->next->wine->count > 0))
			{
				printf("P:#%d signaled P:#%d \n",dinner->id, dinner->next->id);
				
				// to prevent that somebody else signals him twice
				// change the state to ABOUT_TO_EAT
				dinner->next->state = ABOUT_TO_EAT;
				pthread_cond_signal(&(dinner->next->condition_var));
			}
		}
		// TOAST

		// To toast, simply go to sleep inside this critical section
		// no other participant will start eating
		// THESE ALREADY EATING WILL FINISH THEIR CONSUMPTION BEFORE GOING TO LISTEN
		int toast = rand_r(&seed) % 5;
		if(toast == 0)
		{
			int toast_time = 15;
			printf("P:#%d starting TOAST, nobody will start consumption for: %d seconds \n\n",dinner->id,toast_time);
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

	while(table_continue)
	{
		pthread_mutex_lock(&(mutex));

		printf("************************** \n");
		printf("Snack Waiter \n");
		printf("SW: Refilling all bowls of snacks \n");

		int i = 0;
		for(i = 0;i < table->participant_count;i++)
		{
			participant_packet* dinner = dinners[i];
			if( dinner->snack->count != SNACK_MAX)
			{
				dinner->snack->count = SNACK_MAX;
			}

			// signal hungry participants which are eligible for eating
			if(dinner->state == HUNGRY)
			{
				if((dinner->prev->state != EATING) && (dinner->prev->state != ABOUT_TO_EAT)
					&& (dinner->next->state != EATING) && (dinner->next->state != ABOUT_TO_EAT)
					&& (dinner->snack->count > 0) && (dinner->wine->count > 0 ))
				{
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
			printf(KGRN "Currect snack:#%d count: %d\n" RESET, snack->id, snack->count);
			printf(KGRN "Currect wine:#%d count: %d\n" RESET, wine->id, wine->count);
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

	while(table_continue)
	{
		pthread_mutex_lock(&(mutex));
		printf("************************** \n");
		printf("Wine Waiter \n");
		
		// refill all empty bottles
		int j = 0;
		for(j = 0; j < (table->participant_count)/2; j++)
		{
			wine_packet* wine = (table->wines)[j];
			if(wine->count == 0)
			{
				wine->count = WINE_MAX;
				printf("WW: refilling wine:#%d \n",wine->id);
			}
		}
		
		// wake up participants
		int i = 0;
		for(i = 0;i < table->participant_count;i++)
		{
			participant_packet* dinner = dinners[i];

			// signal hungry participants which are eligible for eating
			if(dinner->state == HUNGRY)
			{
				if ((dinner->prev->state != EATING) && (dinner->prev->state != ABOUT_TO_EAT)
					&& (dinner->next->state != EATING) && (dinner->next->state != ABOUT_TO_EAT)
					&& ( dinner->snack->count > 0) && (dinner->wine->count > 0 ))
				{
					printf("WW: signaled P:#%d \n",dinner->id);

					// to prevent that somebody else signals him twice
					// change the state to ABOUT_TO_EAT
					dinner->state = ABOUT_TO_EAT;
					pthread_cond_signal(&(dinner->condition_var));
				}
			}

		}

		// print current state
		for(j = 0; j < (table->participant_count)/2; j++)
		{
			wine_packet* wine = (table->wines)[j];
			snack_packet* snack = (table->snacks)[j];
			printf(KGRN "Currect snack:#%d count: %d\n" RESET,snack->id, snack->count);
			printf(KGRN "Currect wine:#%d count: %d\n" RESET,wine->id, wine->count);
		}

		printf("************************** \n\n");
		pthread_mutex_unlock(&(mutex));
		
		int waiter_sleep = 25;
		sleep(waiter_sleep);
	}
}
