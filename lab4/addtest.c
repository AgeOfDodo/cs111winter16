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

static long long counter = 0;
int opt_yield = 0;
volatile static int spinlock= 0;
static pthread_mutex_t mutex;

int MUTEX=0;    // yield in delete critical section
int SPIN=0; // yield in lookup/length critical section
int ATOMIC=0;

void add(long long *pointer, long long value) {
        long long sum = *pointer + value;
        if (opt_yield) 
            pthread_yield();
        *pointer = sum;
}

void* addreg(int* PTRnum_iterations){
    int i;
    int num_iterations = *(int *)PTRnum_iterations;
        for(i = 0; i < num_iterations; i++) {
        //    printf("i = %d\n", i);
            add(&counter, 1);
        }

        for(i = 0; i < num_iterations; i++) {
             add(&counter, -1);
        }
}

void* addmutex(int* PTRnum_iterations){
    int i;
    int num_iterations = *(int *)PTRnum_iterations;
        for(i = 0; i < num_iterations; ++i) {
            pthread_mutex_lock(&mutex);  
            add(&counter, 1);
            pthread_mutex_unlock(&mutex);  
        }

        for(i = 0; i < num_iterations; ++i) {
            pthread_mutex_lock(&mutex);  
            add(&counter, -1);
            pthread_mutex_unlock(&mutex);  
        }  
}

void* addspin(int* PTRnum_iterations){
    int i;
    int num_iterations = *(int *)PTRnum_iterations;
        for(i = 0; i < num_iterations; ++i) {
            __sync_lock_test_and_set(&spinlock,1);
            add(&counter, 1);
              __sync_lock_release(&spinlock);
        }

        for(i = 0; i < num_iterations; ++i) {
            __sync_lock_test_and_set(&spinlock,1);
             add(&counter, -1);
               __sync_lock_release(&spinlock);
        }
}

void* addswap(int* PTRnum_iterations){
    int i;
    int num_iterations = *(int *)PTRnum_iterations;
    long long * ptr = &counter;
    long long sum;
    int orig;
    for (i = 0; i < num_iterations; ++i) {
        do {
            orig = *ptr;
            sum = orig + 1;
      if (opt_yield) 
        pthread_yield();
        } while(__sync_val_compare_and_swap(ptr, orig, sum)!= orig);
    }
    for (i = 0; i < num_iterations; ++i) {
        do {
            orig = *ptr;
            sum = orig - 1;
      if (opt_yield) 
        pthread_yield();
        } while(__sync_val_compare_and_swap(ptr, orig, sum)!= orig);
    }

}




int main(int argc, char **argv) {

   pthread_mutex_init(&mutex, NULL);
  // c holds return value of getopt_long
  int c;
  long long thread = 1;
  long long iteration = 1;
   struct timespec startTime, endTime; 
   char sync = 'n';
  // Parse options
  while (1) {

      // printf("[profile] begins = %lld\n", (long long) start_s);
    
    int option_index = 0;
    static struct option long_options[] = {
// SUBCOMMAND
   		{"iterations",       optional_argument,        0,  'i' },
        {"threads",       optional_argument,        0,  't' },
        {"yield",       optional_argument,        0,  'y' },
        {"sync",       optional_argument,        0,  'y' },
        {0,0,0,0}
        
    };

    // get the next option
   // printf("Parsing options\n");
    c = getopt_long(argc, argv, "", long_options, &option_index);
   
    // break when there are no further options to parse
    if (c == -1)
      break;

    switch (c) {
    	//SWITCH STATEMENT
    	case 'i':
    		if(optarg != NULL){
    //            printf("interation: %s\n", optarg);
    			iteration = atoi(optarg); 
            }
    	break;

    	case 't':
    		if(optarg != NULL)
    			thread = atoi(optarg);
    	break;

    	case 'y':
    		if(optarg != NULL)
    			opt_yield = 1;
    		//if (atoi(optarg) != 1)
    		//	printf("invalid yield argument\n");
        break;

        case 's':
            if(optarg != NULL){
                sync = optarg[0];
                /*
                switch((int)optarg[0]){ 
                    case 'm':
                            // printf("M\n");   
                        pthread_mutex_init(&mutex, NULL);
                       // MUTEX = 1;
                        break;
                    case 's':
                        //SPIN = 1;
                        break;
                    case 'c':
                       // ATOMIC = 1;
                        break;
                }*/
            }
            else
            //sync = optarg[0];
        break;    
    	

    }
}
    //create threads
	//printf("About to create threads\n");
    pthread_t* thread_array = malloc(sizeof(pthread_t) * thread);
    //printf("Just malloced\n");
    int i;
    clock_gettime(CLOCK_MONOTONIC , &startTime);
    if(thread < 1){
        fprintf(stderr, "ERROR: argument to --threads must be positive\n");
        exit(1);
    }
      if(iteration < 1){
        fprintf(stderr, "ERROR: argument to --iterations must be positive\n");
        exit(1);
    }

    //printf("Just got time\n");
    for(i = 0; i < thread; i++) {
        int ret;
        switch(sync){
            case 'n': //none
            ret = pthread_create((pthread_t * __restrict__) &thread_array[i], NULL, (void * (*)(void *)) addreg, (void *) &iteration); 
            if (ret != 0) { //error handling
                fprintf(stderr, "Error creating thread %d with error code \n", i, ret);
                exit(1);
            }
            break;
            case 's': //spin
            ret = pthread_create((pthread_t * __restrict__) &thread_array[i], NULL, (void * (*)(void *)) addspin, (void *) &iteration); 
            if (ret != 0) { //error handling
                fprintf(stderr, "Error creating thread %d with error code \n", i, ret);
                exit(1);
            }
            break;
            case 'c': //compare and swap
            ret = pthread_create((pthread_t * __restrict__) &thread_array[i], NULL, (void * (*)(void *)) addswap, (void *) &iteration); 
            if (ret != 0) { //error handling
                fprintf(stderr, "Error creating thread %d with error code \n", i, ret);
                exit(1);
            }
            break;
            case 'm': //mutex
            ret = pthread_create((pthread_t * __restrict__) &thread_array[i], NULL, (void * (*)(void *)) addmutex, (void *) &iteration); 
            if (ret != 0) { //error handling
                fprintf(stderr, "Error creating thread %d with error code \n", i, ret);
                exit(1);
            }

            break;
        }



	}

	//wait for all to finish
	//int pthread_join(pthread_t thread, void **retval);
//waits for thread to terminate
	//printf("About to join threads\n");
	for(i = 0; i < thread; i++) {
      //  printf("Number on joining loop: %d\n", i);
		int ret = pthread_join(thread_array[i], NULL);
		if (ret != 0) { //error handling
				fprintf(stderr, "Error joining thread %d\n", i);
				exit(1);
			}
	//need for loop to wait for all
	//also do error handling
	}
	//printf("Succesfully join threads\n");

	//int clock_gettime(clocked_t clk_id , struct timespec* tp) 
 	clock_gettime(CLOCK_MONOTONIC , &endTime);
    /*// straight from the spec
void add(long long *pointer, long long value) {
    long long sum = *pointer + value;
    *pointer = sum;
} */

        //print number of operations
    long long num_ops = thread * iteration * 2;
    printf("%d threads x %d iterations x (add + subtract) = %d\n", thread, iteration, num_ops);
    if(counter != 0){
    	fprintf(stderr, "ERROR: final count = %d\n", counter);
    }


    long long total_time = endTime.tv_nsec - startTime.tv_nsec;
    printf("elapsed time: %d\n", total_time);

    long long avg = total_time / num_ops;
    printf("per operation: %d ns\n", avg);




  // printf("EXit with %d\n",exit_status );
  exit(0);
}