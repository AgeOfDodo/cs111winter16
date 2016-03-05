/* CS111 Winter 2016 Lab1a
See README for further information
TODO:
action handler: change printf to write or whatever
profile
 */

#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <sys/resource.h>
#include <sys/time.h>



#include <pthread.h>
#include <time.h>

#define MAX(a,b) (((a)>(b))?(a):(b))

long long counter = 0;


int opt_yield;

void add(long long *pointer, long long value) {
    long long sum = *pointer + value;
        if (opt_yield)
            pthread_yield();
        *pointer = sum;
}




void* threadfunc(int num_iterations){
	int i;
	for(i = 0; i < num_iterations; ++i) {
		add(&counter, 1);
	}

	for(i = 0; i < num_iterations; ++i) {
		add(&counter, -1);
	}
}



int main(int argc, char **argv) {


  // c holds return value of getopt_long
  int c;
  int thread = 1;
  int iteration = 1;
   struct timespec startTime, endTime; 

  // Parse options
  while (1) {

      // printf("[profile] begins = %lld\n", (long long) start_s);
    
    int option_index = 0;
    static struct option long_options[] = {
// SUBCOMMAND
   		{"iterations",       optional_argument,        0,  'i' },
        {"threads",       optional_argument,        0,  't' },
        {"yield",       optional_argument,        0,  'y' },
        {0,0,0,0}
        
    };

    // get the next option
    c = getopt_long(argc, argv, "", long_options, &option_index);
   
    // break when there are no further options to parse
    if (c == -1)
      break;

    switch (c) {
    	//SWITCH STATEMENT
    	case 'i':
    		if(optarg != NULL)
    			iteration = atoi(optarg); 
    	break;

    	case 't':
    		if(optarg != NULL)
    			thread = atoi(optarg);
    	break;

    	case 'y':
    		if(optarg != NULL)
    			opt_yield = 1;
    		if (atoi(optarg) != 1)
    			printf("invalid yield argument\n");
    	break;

    }
}
    //create threads
    pid_t* thread_array = malloc(sizeof(pid_t) * thread);
    int i;
    clock_gettime(CLOCK_MONOTONIC , &startTime);
    for(i = 0; i < thread; i++) {
		int ret = pthread_create((pthread_t * __restrict__) &thread_array[i], NULL, (void * (*)(int)) threadfunc, (void *) &iteration);  //to create thread
			if (ret != 0) { //error handling
				fprintf(stderr, "Error creating thread %d\n", i);
				exit(1);
			}
	}

	//wait for all to finish
	//int pthread_join(pthread_t thread, void **retval);
//waits for thread to terminate
	for(i = 0; i < thread; i++) {
		int ret = pthread_join(thread_array[i], NULL);
		if (ret != 0) { //error handling
				fprintf(stderr, "Error joining thread %d\n", i);
				exit(1);
			}
	//need for loop to wait for all
	//also do error handling
	}

	//int clock_gettime(clocked_t clk_id , struct timespec* tp) 
 	clock_gettime(CLOCK_MONOTONIC , &endTime);
    /*// straight from the spec
void add(long long *pointer, long long value) {
    long long sum = *pointer + value;
    *pointer = sum;
} */

        //print number of operations
    long num_ops = thread * iteration * 2;
    printf("%d threads x %d iterations x (add + subtract) = %d\n", thread, iteration, num_ops);
    if(counter != 0){
    	fprintf(stderr, "ERROR: final count = %d\n", counter);
    }


    int total_time = endTime.tv_nsec - startTime.tv_nsec;
    printf("elapsed time: %d\n", total_time);

    int avg = total_time / num_ops;
    printf("per operation: %d ns\n", avg);




  // printf("EXit with %d\n",exit_status );
  exit(0);
}