/*

NO FORK RELEASE MECHANISM YET
*/

/*
https://stackoverflow.com/questions/14635827/use-read-to-take-user-input-instead-of-scanf
https://stackoverflow.com/questions/15102992/what-is-the-difference-between-stdin-and-stdin-fileno
*/

/*

Compile:
gcc --std=c99 -Wall -g -Werror -lpthread dinphil.c -o dinphil

*/
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>

// Global variables
#define NUM_OF_PHIL 5


// Initialize philosophers' states
enum philState_t { THINKING=0,EATING=1,HUNGRY=2}; // Group state THINKING and HUNGRY to 0 
enum philState_t phil_state [ NUM_OF_PHIL ];
enum forkState_t { FREE, TAKEN }; 
enum forkState_t fork_state [ NUM_OF_PHIL ];

pthread_cond_t fork_cond [ NUM_OF_PHIL ];
pthread_cond_t think_request_cond [ NUM_OF_PHIL ]; 
pthread_cond_t eat_request_cond [ NUM_OF_PHIL ] ;

pthread_mutex_t lock;


typedef struct {
	int position;
	int has_left_fork;
	int has_right_fork;
} philosopher_t;


//void initPhil();
void printStateOfPhil(enum philState_t phil_state[]); // make sure this is atomic
void *philRun(void *arg);
void pickUpForks(philosopher_t *phil);
void returnForks(philosopher_t* phil);

void test(philosopher_t *phil);
void printStateOfForks (enum forkState_t fork_state[]);
int shouldTakeLeftForkFirst(int id);

int main() {
	
	// Create threads
	pthread_mutex_init ( &lock, NULL );
	pthread_t threads[NUM_OF_PHIL];
	

	//philosopher_t *phils[ NUM_OF_PHIL ] = malloc(sizeof(philosopher_t) * NUM_OF_PHIL);
	
	for(int i = 0; i < NUM_OF_PHIL; i++) {
		philosopher_t *phil = malloc(sizeof(philosopher_t));
	   
	    phil->position = i;
	    phil->has_left_fork = 0;
	    phil->has_right_fork = 0;

	    phil_state[i] = THINKING;
	    fork_state[i] = FREE; 

	    pthread_cond_init ( &eat_request_cond [ i ], NULL );
	    pthread_cond_init ( &think_request_cond [ i ], NULL );
	    pthread_cond_init ( &fork_cond [ i ], NULL );
	    
	    pthread_create(&threads[i], NULL, philRun, (void *) phil);  
	}



	// for(int i = 0; i < NUM_OF_PHIL; i++) {
	// 	pthread_cond_init ( &eat_request_cond [ i ], NULL );
	//     pthread_cond_init ( &think_request_cond [ i ], NULL );
	//     pthread_cond_init ( &fork_cond [ i ], NULL );
	    
	//     pthread_create(&threads[i], NULL, philRun, (void *) phil);
	// } 

	char input[128]; 	    
		
	while(1) {

		// User input handling	 	    
	    read(0, &input, sizeof(input));

		if (input[0] == 'P') {
			
			pthread_mutex_lock (&lock);
			
			printStateOfPhil(phil_state); 
			
			pthread_mutex_unlock (&lock);
	
		} else if (input[0] == 'E') {
			int input_num = input[2] - '0';

			pthread_mutex_lock (&lock);
			
			// if state is EATING we skip and don't change the state
			if (phil_state[ input_num ] == THINKING) {	
				//if current state is THINKING we change to HUNGRY state
				phil_state[ input_num ] = HUNGRY; 
			}
			// Let the thread know
			pthread_cond_signal( &eat_request_cond [input_num]);
			
			pthread_mutex_unlock (&lock);	
		
		} else if (input[0] == 'T') {
			int input_num = input[2] - '0'; 

			pthread_mutex_lock (&lock);
			
			phil_state[ input_num ] = THINKING;
			pthread_cond_signal( &think_request_cond [input_num]);
			
			pthread_mutex_unlock (&lock);	
		
		} else if (input[0] == '!') {
			exit(0);
		} else {
			// do nothing
		}
	}
	return 0;
}


void *philRun(void *arg) {
	
	philosopher_t* phil = (philosopher_t *) arg;
	

	int id = phil->position;
	
	printf("Thread Phil %d created.\n", id);
	
	while(1) {
		
		pthread_mutex_lock ( &lock );
		//pthread_cond_wait ( & eat_request_cond[ id ], &lock);
		
		while ( phil_state [ id ] ==  THINKING) {
			// condition wait if no user input
			
			pthread_cond_wait ( & eat_request_cond[ id ], &lock);

			printf("This is phil %d. I am now hungry\n", id);
	
			
		}
		pthread_mutex_unlock ( &lock );
		

		
		pickUpForks ( phil);
		

		

		pthread_mutex_lock ( &lock );
		// cannot change to THINKING unless Phil[id] is serviced after E request
		while ( phil_state [ id ] != THINKING) { 
			// condition wait if no user input
			pthread_cond_wait ( & think_request_cond[ id ], &lock);

		}
		pthread_mutex_unlock ( &lock );



		returnForks(phil);


	}

	pthread_exit(0);
}


void pickUpForks(philosopher_t *phil){
	
	int id = phil->position;
	int right_fork = (id + 1 ) % NUM_OF_PHIL;
	int left_fork = id;
	
	test(phil);
	

	pthread_mutex_lock(&lock);
	printStateOfForks(fork_state);
	printStateOfPhil(phil_state);      
	pthread_mutex_unlock(&lock);

	pthread_mutex_lock ( &lock );
	
	while ( phil_state [ id ] != EATING ) {
		
		if (shouldTakeLeftForkFirst(id)) {
		
			pthread_cond_wait ( &fork_cond [ left_fork ], &lock );
			pthread_cond_wait ( &fork_cond [ right_fork ], &lock );
		
		} else {
			pthread_cond_wait ( &fork_cond [ right_fork ], &lock );
			pthread_cond_wait ( &fork_cond [ left_fork ], &lock );

		} 			
	}
	
	pthread_mutex_unlock ( &lock );

	printf("wasn't wating\n");
	pthread_mutex_lock(&lock);
	printStateOfForks(fork_state);
	printStateOfPhil(phil_state);
	pthread_mutex_unlock(&lock);


}

void test(philosopher_t *phil) {
	int id = phil->position;
	
	int right_fork = (id + 1 ) % NUM_OF_PHIL;
	int left_fork = id;

	
	if( shouldTakeLeftForkFirst(id) ) {
		pthread_mutex_lock(&lock);
		
		if ( (fork_state[left_fork] == FREE) && ( phil_state[id] == HUNGRY) ) {
			fork_state[left_fork] = TAKEN;
			phil->has_left_fork = 1;

			pthread_cond_signal(&fork_cond[left_fork]);
		}

		if ( ( phil-> has_left_fork == 1) && ( fork_state[right_fork] == FREE) && (phil_state[id] == HUNGRY) ) {
			fork_state[right_fork] = TAKEN;
			phil->has_right_fork = 1;

			phil_state[id] = EATING;
			
			pthread_cond_signal(&fork_cond[right_fork]);	
		}
		
		pthread_mutex_unlock(&lock);

	} else {

		pthread_mutex_lock(&lock);

		if ( (fork_state[right_fork] == FREE) && ( phil_state[id] == HUNGRY) ) {
			fork_state[right_fork] = TAKEN;
			phil->has_right_fork = 1;

			pthread_cond_signal(&fork_cond[right_fork]);
		}

		if ( ( phil-> has_right_fork == 1) && ( fork_state[left_fork] == FREE) && (phil_state[id] == HUNGRY) ) {
			fork_state[left_fork] = TAKEN;

			phil->has_left_fork = 1;

			phil_state[id] = EATING;

			pthread_cond_signal(&fork_cond[left_fork]);	
		}

		pthread_mutex_unlock(&lock);

	}
}



void returnForks(philosopher_t* phil) {
	int id = phil-> position;
	//int num = NUM_OF_PHIL;
	
	pthread_mutex_lock ( &lock );						
	phil_state [ id ] = THINKING;
	pthread_mutex_unlock ( &lock );

	// call to check for other philosphers to avoid starvation
	//test ( ( id + num - 1 ) % num );					
	//test ( ( id + 1 ) % num );
}

int shouldTakeLeftForkFirst(int id) {
	if (id != NUM_OF_PHIL-1) {
		return 1;
	} 
	return 0;
}

void printStateOfForks (enum forkState_t fork_state[]) {
	int num_of_spaces = NUM_OF_PHIL - 1; 
	int output_size = NUM_OF_PHIL + num_of_spaces + 1;
	char output[output_size]; 
	
	char *msg="fork status: ";

	output[output_size-1] = '\n';
	for (int i = 0; i < output_size-1; i++) {
		if (i%2 == 0) {
			output[i] = fork_state[i/2] + '0'; // convert int to char
		} else {
			output[i] = ' ';
		}	
	}

	write (1, msg, strlen(msg));
	write (1, output, sizeof(output));	


}





void printStateOfPhil(enum philState_t phil_state[] ) {
	
	int num_of_spaces = NUM_OF_PHIL - 1; 
	int output_size = NUM_OF_PHIL + num_of_spaces + 1;
	char output[output_size]; 

	char *msg="phil status_: ";
	
	output[output_size-1] = '\n';

	for (int i = 0; i < output_size-1; i++) {
		
		if (i%2 == 0) {		
			 output[i] = phil_state[i/2]+ '0'; // convert int to char
		} else {
			output[i] = ' ';
		}	
	}
	write (1, msg, strlen(msg));
	write (1, output, sizeof(output));	
	

}












