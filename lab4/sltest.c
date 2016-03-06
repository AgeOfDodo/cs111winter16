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

long long counter = 0;



struct SortedListElement {
	struct SortedListElement *prev;
	struct SortedListElement *next;
	const char *key;
};
typedef struct SortedListElement SortedList_t;
typedef struct SortedListElement SortedListElement_t;

void SortedList_insert(SortedList_t *list, SortedListElement_t *element){
	SortedListElement_t *p = list;
	SortedListElement_t *n = list->next;
	while(n != list){
		if(strcmp(element->key, n->key) <= 0)
			break;
	}
	p = n;
	n = n->next;
	element->prev = p;
	element->next = n;
	p->next = element;
	n->prev = element;
}


int SortedList_delete( SortedListElement_t *element){
	SortedListElement_t *n = element->next;
	SortedListElement_t *p = element->prev;
	if(n->prev != element)
		return -1;
	if(p->next != element)
		return 1;
	n->prev = p;
	p->next = n;
	element->next = NULL;
	element->prev = NULL;

	return 0;
}


SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key){
	if(list == NULL)	
		return NULL;
	SortedList_t* head = list;
	do{
		if(strcmp(list->key, key) == 0)
			return list;
		list = list->next;
	} while (head != list);
	return NULL;
}

int SortedList_length(SortedList_t *list){
	if(list == NULL)	
		return 0;
	int retval = 0;
	SortedList_t* head = list;
	do{
		retval++;
		list = list->next;
	} while (head != list);
	return retval;
}

/**
 * variable to enable diagnositc calls to pthread_yield
 */
extern int opt_yield;
#define	INSERT_YIELD	0x01	// yield in insert critical section
#define	DELETE_YIELD	0x02	// yield in delete critical section
#define	SEARCH_YIELD	0x04	// yield in lookup/length critical section

SortedList_t* list = NULL;
SortedListElement_t* elements = NULL;
int nElements = 0;
int iterations = 0;
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

void* threadfunc(int index){
	int length;
	int i;
	// insert each element 
	for(i = 0; i < iterations; ++i) {
		SortedList_insert(list, &elements[index*nElements + i]);
	}
	// get length
	length = SortedList_length(list);
	
	// look up/ delete
	for(i = 0; i < iterations; ++i) {
		SortedListElement_t* target=  SortedList_lookup(list, elements[index*nElements + i].key);	
		if(target == NULL)
			printf("should never be here.\b");
			continue;
		SortedList_delete(target);
	}

}

int main(int argc, char** argv){

  	// c holds return value of getopt_long
	int c;
	int thread = 1;
	int iteration = 1;
	char yield = NULL;
	struct timespec startTime, endTime; 

	  // Parse options
	while (1) {
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
	    			yield = optarg;
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
	nElements = thread * iteration;
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
    pthread_t * thread_array = malloc(sizeof(pthread_t)* thread);
    
    // start time
    clock_gettime(CLOCK_MONOTONIC , &startTime);

    for(i = 0; i < thread; i++) {
		int ret = pthread_create(&thread_array[i], NULL, threadfunc, (void *) &i);  //to create thread
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
    long num_ops = thread * iteration * 3 + 1;
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