// cs111 lab4 part 2

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
#include "SortedList.h"

static pthread_mutex_t mutex;
volatile static int spinlock= 0;


int	MUTEX=0;	// yield in delete critical section
int SPIN=0;	// yield in lookup/length critical section

long long counter = 0;
int opt_yield = 0;

// for debugging purpose
void SortedList_display(SortedList_t *list){
	if(list == list->next){
		printf("list is empty.\n");
		return;
	}
	SortedList_t* head = list;
	list = list->next;
	int i = 0;
	do{
		printf("list[%d]= %c\n",i, (list->key)[i]);
		list = list->next;
		i++;
	} while (head != list);
}



SortedList_t* list = NULL;
SortedListElement_t* elements = NULL;
int nElements = 0;
int iterations = 1;
// generate random keys with length 32
char* gen_random(char* s) {
	// char s[33];
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    int i;
    for (i = 0; i < 32; ++i) {
        s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
    }
    s[32] = 0;
    return s;
}

void* threadfunc(void* ind){
	int index = *((int*) ind);
	int length;
	int i;
	// insert each element 
	for(i = 0; i < iterations; ++i) {
		if(MUTEX){
			pthread_mutex_lock(&mutex);
			printf("grabbed mutex for insert\n");
			SortedList_insert(list, &elements[index*nElements + i]);
			pthread_mutex_unlock(&mutex);
			printf("released mutex for insert\n");
		}
		else if(SPIN){
			while(__sync_lock_test_and_set(&spinlock,1)){
				SortedList_insert(list, &elements[index*nElements + i]);	
			}
			__sync_lock_release(&spinlock);
		}else{
			SortedList_insert(list, &elements[index*nElements + i]);
		}		
	}

	// SortedList_display(list);
	// get length
	if(MUTEX){
		pthread_mutex_lock(&mutex);
		length = SortedList_length(list);
		pthread_mutex_unlock(&mutex);
	}else if(SPIN){
		while(__sync_lock_test_and_set(&spinlock,1)){
			SortedList_length(list);	
		}
		__sync_lock_release(&spinlock);
	}else{
		length = SortedList_length(list);
	}
	// printf("length = %d\n", length);
	// look up/ delete
	for(i = 0; i < iterations; ++i) {
		SortedListElement_t* target;
		if(MUTEX){
			pthread_mutex_lock(&mutex); 	
			printf("mutex\n");
			target = SortedList_lookup(list, elements[index*nElements + i].key);
			SortedList_delete(target);
			if(target == NULL){
				printf("should never be here.\b");
				continue;
			}
			pthread_mutex_unlock(&mutex);
		}else if(SPIN){
			while(__sync_lock_test_and_set(&spinlock,1)){
				target=  SortedList_lookup(list, elements[index*nElements + i].key);
				SortedList_delete(target);
				if(target == NULL){
					printf("should never be here.\b");
					continue;
				}
			}
			__sync_lock_release(&spinlock);
		}else{
			target=  SortedList_lookup(list, elements[index*nElements + i].key);	
			SortedList_delete(target);
			if(target == NULL){
				printf("should never be here.\b");
				continue;
			}
		}

	}

	// SortedList_display(list);
	// length = SortedList_length(list);
}

int main(int argc, char** argv){

  	// c holds return value of getopt_long
	int c;
	int nThreads = 1;
	char* yield = NULL;
	struct timespec startTime, endTime; 
	  // Parse options
	while (1) {
	    int option_index = 0;
	    static struct option long_options[] = {
	// SUBCOMMAND
	   		{"iterations",       optional_argument,        0,  'i' },
        	{"threads",       optional_argument,        0,  't' },
        	{"yield",       optional_argument,        0,  'y' },
        	{"sync",		optional_argument,		0,	's'},
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
	    			iterations = atoi(optarg); 
	    	break;

	    	case 't':
	    		if(optarg != NULL)
	    			nThreads = atoi(optarg);
	    	break;

	    	case 'y':
	    		if(optarg != NULL)
	    			yield= optarg;
	    		//  012345
	    			int i = 0;
	    			int run = 1;
	    			while(run){
	    				switch((int)yield[i]){
	    					case 'i':
	    						opt_yield |= INSERT_YIELD;
	    						printf("y=insert\n");
	    						break;
	    					case 'd':
	    						opt_yield |= DELETE_YIELD;
	    						printf("y=delte\n");
	    						break;
	    					case 's':
	    						opt_yield |= SEARCH_YIELD;
	    						printf("y=search\n");
	    						break;
	    					default:
	    						run = 0;
	    				}
	    				i++;
	    			}
	    		break;
	    	case 's':
	    		if(optarg != NULL){
    				switch((int)optarg[0]){
    					case 'm':
    						printf("M\n");	
							pthread_mutex_init(&mutex, NULL);
    						MUTEX = 1;
    						break;
    					case 's':
    						SPIN = 1;
    						break;
	    			}
	    		}
	    		break;
   		}

	}

	// initialize an empty list( with dummy)
 	SortedList_t l;
 	list = &l;
 	
	list->key = NULL;
	list->prev = list;
	list->next = list;

	// create and initializes the required number
	nElements = nThreads * iterations;
	elements = malloc(sizeof(SortedListElement_t) * nElements);
	int i = 0;
	for(i = 0 ; i != nElements; i++){
		char s[32];
		gen_random(s);
		elements[i].key = s;
		elements[i].next = NULL;
		elements[i].prev = NULL;
	}

    //create threads
    pthread_t * thread_array = malloc(sizeof(pthread_t)* nThreads);
    
    // start time
    clock_gettime(CLOCK_MONOTONIC , &startTime);

    for(i = 0; i < nThreads; i++) {
		int ret = pthread_create(&thread_array[i], NULL, threadfunc, (void *) &i);  //to create thread
			if (ret != 0) { //error handling
				fprintf(stderr, "Error creating thread %d\n", i);
				exit(1);
			}
	}

	//wait for all to finish
	//int pthread_join(pthread_t thread, void **retval);
	//waits for thread to terminate
	for(i = 0; i < nThreads; i++) {
		int ret = pthread_join(thread_array[i], NULL);
		if (ret != 0) { //error handling
				fprintf(stderr, "Error joining thread %d\n", i);
				exit(1);
		}
	//need for loop to wait for all
	//also do error handling
	}


	// check th length of the list
	//int clock_gettime(clocked_t clk_id , struct timespec* tp) 
 	clock_gettime(CLOCK_MONOTONIC , &endTime);
	int finalLength = SortedList_length(list);
	if( finalLength != 0) {
    	fprintf(stderr, "Error! Final list length is not zero!\n");
    	exit(1);
    }

// print

        //print number of operations
    long num_ops = nThreads * iterations * 3 + 1;
    printf("%d threads x %d iterations x (ins + lookup/del) x (100/2 avg len) = %d operations\n", nThreads, iterations, num_ops);
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
