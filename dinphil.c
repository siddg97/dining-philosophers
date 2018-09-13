/*
Interactive Dining Philosopher using resource hierarchy solution
Name: Jessie Tran
Student ID: 301262354
pmtran@sfu.ca
Course: CMPT300
File: dinphil.c

Compile:

gcc --std=c99 -Wall -g -Werror dinphil.c -o dinphil -lpthread

*/

/*
Helpful Sources:
Textbook chap 5 pg 228 (Monitor pseudocode), pg 252 for pthread_cond_t
Dr. Brian Fraser videos
https://stackoverflow.com/questions/42720131/multiple-locks-with-mutex-and-the-possibility-of-a-deadlock#
https://stackoverflow.com/questions/12551341/when-is-a-condition-variable-needed-isnt-a-mutex-enough
https://stackoverflow.com/questions/14635827/use-read-to-take-user-input-instead-of-scanf
https://stackoverflow.com/questions/15102992/what-is-the-difference-between-stdin-and-stdin-fileno
*/

#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>

#define NUM_OF_PHIL 5 

enum philState_t { THINKING=0,EATING=1,HUNGRY=2}; 
enum philState_t phil_state [ NUM_OF_PHIL ];

enum forkState_t { FREE, TAKEN }; 
enum forkState_t fork_state [ NUM_OF_PHIL ];

pthread_cond_t fork_cond [ NUM_OF_PHIL ];
pthread_cond_t think_request_cond [ NUM_OF_PHIL ]; 
pthread_cond_t eat_request_cond [ NUM_OF_PHIL ] ;

pthread_mutex_t lock;


typedef struct philosopher {
	int position;
	int has_left_fork;
	int has_right_fork;

	struct philosopher* left_neighbor;
	struct philosopher* right_neighbor;
} philosopher_t;


void initPhil(philosopher_t ** phils);
void freePhils (philosopher_t **phils);

void *philRun(void *arg);
void pickUpForks(philosopher_t *phil);
void returnForks(philosopher_t *phil);
void test(philosopher_t *phil);
int shouldTakeLeftForkFirst(int id);

void printStateOfPhil(); 
void printStateOfForks ();
void printStateVerbose();




//--------------------------------------------------------------------------------
// MAIN
//--------------------------------------------------------------------------------
int main() {
	
	// Create philosophers
	philosopher_t **phils = malloc(sizeof(philosopher_t *) * NUM_OF_PHIL );
	initPhil(phils);
	
	// Create threads, lock, and condition variables
	pthread_mutex_init ( &lock, NULL );
	pthread_t threads[NUM_OF_PHIL];

	for(int i = 0; i < NUM_OF_PHIL; i++) {
		pthread_cond_init ( &eat_request_cond [ i ], NULL );
	    pthread_cond_init ( &think_request_cond [ i ], NULL );
	    pthread_cond_init ( &fork_cond [ i ], NULL );
	    
	    pthread_create(&threads[i], NULL, philRun, (void *) phils[i]);
	    
	} 
 
	// User input handling	 	    
	char input[128]; 	    
		
	while(1) {

	    read(0, &input, sizeof(input));

		if (input[0] == 'P') {
				
			printStateOfPhil(0);
			//printStateVerbose(); // to see fork state and which phil is in hungry state
				
		} else if (input[0] == 'E') {
			int input_num = input[2] - '0';

			pthread_mutex_lock (&lock);
			// if state is EATING or HUNGRY we skip and don't change the state
			if (phil_state[ input_num ] == THINKING) {	
				//if current state is THINKING we change to HUNGRY state
				phil_state[ input_num ] = HUNGRY; 
				pthread_cond_signal( &eat_request_cond [input_num]);
			}

			pthread_mutex_unlock (&lock);	
		
		} else if (input[0] == 'T') {
			int input_num = input[2] - '0'; 
			pthread_mutex_lock (&lock);
			
			// if state is THINKING or HUNGRY we skip and don't change the state
			// Phil can't THINK if they are HUNGRY
			// Meaning if Phil has 1 fork while waiting for another. Phil can't release that fork
			// Phil must be able to EAT before they can process user request to THINK

			if ( phil_state [input_num] == EATING) {
				phil_state[ input_num ] = THINKING;
				pthread_cond_signal( &think_request_cond [input_num]);
			}
			
			pthread_mutex_unlock (&lock);	
			
		} else if (input[0] == '!') {
			break;			
		} else {
			// do nothing
		}
	}


	//clean up
	for(int i = 0; i < NUM_OF_PHIL; i++) {
		pthread_cond_destroy ( &eat_request_cond [ i ]);
	    pthread_cond_destroy ( &think_request_cond [ i ]);
	    pthread_cond_destroy ( &fork_cond [ i ]);
		
		pthread_cancel(threads[i]);
	} 
	
	pthread_mutex_destroy(&lock);
	
	freePhils(phils);
	//exit(0);
	return 0;
}

//--------------------------------------------------------------------------------
// THREAD FUNCTIONS
//--------------------------------------------------------------------------------
void *philRun(void *arg) {
	
	philosopher_t* phil = (philosopher_t *) arg;
	int id = phil->position;
	
	while( 1 ) {		

		pthread_mutex_lock ( &lock );
				
		while ( phil_state [ id ] ==  THINKING ) {
			// Wait for eating request
			pthread_cond_wait ( & eat_request_cond[ id ], &lock);
		}
		pthread_mutex_unlock ( &lock );
		
		// Got signaled from main thread. Proceed to pickup forks.
		pickUpForks ( phil );

		pthread_mutex_lock ( &lock );
		while ( phil_state [ id ] == EATING ) { 
			//Wait for thinking request after Phil has been served
			pthread_cond_wait ( & think_request_cond[ id ], &lock);
		}
		
		pthread_mutex_unlock ( &lock );
		
		returnForks( phil);
	}
	
	pthread_exit(0);
}


void pickUpForks(philosopher_t *phil){
	
	int id = phil->position;
	int right_fork = (id + 1 ) % NUM_OF_PHIL;
	int left_fork = id;
	
	// Test() will assign forks if both are free right away. 
	// If not, it will assign at least 1 fork. We move on to waiting.
	test(phil);

	pthread_mutex_lock ( &lock );
	
	if ( phil_state [ id ] == HUNGRY ) {
		
		//Except for the highest phil (4) everyone picks up left fork first
		if (shouldTakeLeftForkFirst(id)) {
			//Wait for left fork
			while ( !phil->has_left_fork) {			
				pthread_cond_wait ( &fork_cond [ left_fork ], &lock );
			}
			//Wait for right fork
			while ( !phil->has_right_fork) {
				pthread_cond_wait ( &fork_cond [ right_fork ], &lock );
			}

		// Phil with highest number has to pick up right fork first
		} else { 
			// Wait for right fork
			while ( !phil->has_right_fork ) {
				pthread_cond_wait ( &fork_cond [ right_fork ], &lock );
			}
			// Wait for left fork	
			while ( !phil->has_left_fork ) {
				pthread_cond_wait ( &fork_cond [ left_fork ], &lock );
			}
		
		} 			
	}
	
	pthread_mutex_unlock ( &lock );

}

void test(philosopher_t *phil) {

	int id = phil->position;
	int right_fork = (id + 1 ) % NUM_OF_PHIL;
	int left_fork = id;
	
	// Except for the highest numbered philosopher.
	// everyone picks up left fork first
	if( shouldTakeLeftForkFirst(id) ) {
		pthread_mutex_lock(&lock);
		// Test for phil state and fork state. Only wait for forks if they are HUNGRY	
		if ( (fork_state[left_fork] == FREE) && ( phil_state[id] == HUNGRY) ) {
			// Pick up left fork
			fork_state[left_fork] = TAKEN;
			phil->has_left_fork = 1;
			// Wake up cond_wait in pickUpForks() 
			pthread_cond_signal(&fork_cond[left_fork]);
		}
		// Pick up right fork if Phil already has left fork
		if ( ( phil-> has_left_fork == 1) && ( fork_state[right_fork] == FREE) && (phil_state[id] == HUNGRY) ) {
			fork_state[right_fork] = TAKEN;
			phil->has_right_fork = 1;
			
			//Both forks obtained. Change state.
			phil_state[id] = EATING;
			// Wake up cond_wait in pickUpForks() 
			pthread_cond_signal(&fork_cond[right_fork]);	
		}
		
		pthread_mutex_unlock(&lock);

	// For highest numbered philosopher, right fork first.
	} else {
		pthread_mutex_lock(&lock);

		if ( (fork_state[right_fork] == FREE) && ( phil_state[id] == HUNGRY) ) {
			fork_state[right_fork] = TAKEN;
			phil->has_right_fork = 1;

			pthread_cond_signal(&fork_cond[right_fork]);
		}
		// Pick up left fork if Phil already has right fork
		if ( ( phil-> has_right_fork == 1) && ( fork_state[left_fork] == FREE) && (phil_state[id] == HUNGRY) ) {
			// Pick up fork
			fork_state[left_fork] = TAKEN;
			phil->has_left_fork = 1;

			//Both forks obtained. Change state.
			phil_state[id] = EATING;

			// Wake up pickUpForks()
			pthread_cond_signal(&fork_cond[left_fork]);	
		}

		pthread_mutex_unlock(&lock);

	}
}



void returnForks(philosopher_t *phil) {
	int id = phil->position;

	pthread_mutex_lock ( &lock );						
	// Free right fork
	fork_state[ (id + 1 ) % NUM_OF_PHIL] = FREE;	
	// Free left fork
	fork_state[id] = FREE;
	
	// Reset has_fork conditions
	phil->has_right_fork=0;
	phil->has_left_fork=0;
	pthread_mutex_unlock ( &lock );
	
	// Free neighbors in case they are HUNGRY
	test ( phil->left_neighbor );	
	test ( phil->right_neighbor);
	
	//printStateVerbose();

}

int shouldTakeLeftForkFirst(int id) {
	if (id != NUM_OF_PHIL-1) {
		return 1;
	} 
	return 0;
}

//--------------------------------------------------------------------------------
// PRINTING FUNCTIONS
//--------------------------------------------------------------------------------

void printStateOfForks () {
	int num_of_spaces = NUM_OF_PHIL - 1; 
	int output_size = NUM_OF_PHIL + num_of_spaces + 1;
	//Create buffer
	char output[output_size]; 
	
	// Fill buffer
	output[output_size-1] = '\n';

	pthread_mutex_lock(&lock);
	for (int i = 0; i < output_size-1; i++) {
		if (i%2 == 0) {
			output[i] = fork_state[i/2] + '0'; // convert int to char
		} else {
			output[i] = ' ';
		}	
	}
	pthread_mutex_unlock(&lock);

	write (1, output, sizeof(output));	
}


void printStateOfPhil(int hungry_allowed) {
	
	int num_of_spaces = NUM_OF_PHIL - 1; 
	int output_size = NUM_OF_PHIL + num_of_spaces + 1;
	
	// Create buffer
	char output[output_size]; 
	// Fill last entry with new line
	output[output_size-1] = '\n';

	//Fill buffer
	pthread_mutex_lock(&lock);

	for (int i = 0; i < output_size-1; i++) {

		int current = phil_state[i/2];

		if (i%2 == 0) {
			// Number 2 represents HUGNRY state
			// If not allowed such state, print 0 instead
			if (!hungry_allowed) {
				if (current == 2) {
					output[i] = '0';		
				} else {
				 output[i] = current + '0'; 
				}
			} else {
				output[i] = current + '0';
			}
		} else {
			output[i] = ' ';
		}	
	}
	pthread_mutex_unlock(&lock);

	write (1, output, sizeof(output));	
	
}

void printStateVerbose() {
	char *msg2="fork status: ";
	write (1, msg2, strlen(msg2));
	printStateOfForks();		

	char *msg1="phil status_: ";
	write (1, msg1, strlen(msg1));
	printStateOfPhil(1);
}

//--------------------------------------------------------------------------------
// HELPER FUNCTIONS
//--------------------------------------------------------------------------------

void initPhil(philosopher_t ** phils) {
		for(int i = 0; i < NUM_OF_PHIL; i++) {	   	
	   	phils[i] =  malloc(sizeof(philosopher_t));
	    
	    phils[i]->position = i;
	    phils[i]->has_left_fork = 0;
	    phils[i]->has_right_fork = 0;

	    phil_state[i] = THINKING;
	    fork_state[i] = FREE; 
	}

	for(int i = 0; i < NUM_OF_PHIL; i++) {
	  
	    // Assign left neighbor
	    if ( i == 0 ) {
	    	phils[i]->left_neighbor = phils[NUM_OF_PHIL-1];
	    } else {
	     	phils[i]->left_neighbor =  phils[i-1];
	    }	 
	    
	    //Assign right neighbor
	    if (i == NUM_OF_PHIL- 1) {
	    	phils[i]->right_neighbor = phils[0];
	    } else {
	    	phils[i]->right_neighbor = phils[i+1];
	    }	
	   
	}
}

// Free dynamically allocated mem
void freePhils (philosopher_t **phils) {	
	for(int i = 0; i < NUM_OF_PHIL; i++) {
		philosopher_t* ptr = phils[i];
		free(ptr);
	}
	free(phils);
}












