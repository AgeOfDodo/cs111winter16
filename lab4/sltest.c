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

static pthread_mutex_t *mutex;
static int* spinlock;
static pthread_mutex_t len_mutex;
static int len_spinlock;

int	MUTEX=0;	// yield in delete critical section
int SPIN=0;	// yield in lookup/length critical section

long long counter = 0;
int opt_yield = 0;


// for lists
SortedList_t* list = NULL;
SortedListElement_t* elements = NULL;
int nElements = 0;
int iterations = 1;
int nThreads = 1;
char** keys = NULL;
int nLists = 1;



void SortedList_insert(SortedList_t *list, SortedListElement_t *element){
	// printf("insert %s\t", element->key);
	SortedListElement_t *p = list;
	SortedListElement_t *n = list->next;
	while(n != list){
		if(strcmp(element->key, n->key) <= 0)
			break;
		n = n->next;
	}
	if(opt_yield & INSERT_YIELD){
		pthread_yield();			
	}
	p = n->prev;
	element->prev = p;
	element->next = n;
	p->next = element;
	n->prev = element;
	// printf("p=%s, n=%s\n", p->key, n->key);
}



int SortedList_delete( SortedListElement_t *element){
	// printf("delete\n");
	SortedListElement_t *n = element->next;
	SortedListElement_t *p = element->prev;
	if(n->prev != element)
		return -1;
	if(p->next != element)
		return 1;
	
	if(opt_yield & DELETE_YIELD){
		pthread_yield();			
	}

	n->prev = p;
	p->next = n;
	element->next = NULL;
	element->prev = NULL;

	return 0;
}


SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key){
	// printf("lookup\n");

	if(list == list->next)	
		return NULL;
	SortedList_t* head = list;
	list = list->next;	
	while (head != list){
		if(strcmp(list->key, key) == 0)
			if(opt_yield & SEARCH_YIELD){
				pthread_yield();			
			}
			return list;
		list = list->next;
	} 
	return NULL;
}

int SortedList_length(SortedList_t *lists){
	int i = 0;
	int finalRet = 0;
	for(i = 0; i != nLists; i++){
		SortedList_t* list= &lists[i];
		if(list == list->next)	
			continue;

		int retval = -1;	//dummy node doesn't count
		SortedList_t* head = list;
		do{
			retval++;
			list = list->next;
			if(opt_yield & SEARCH_YIELD){
					pthread_yield();			
			}
		} while (head != list);
		finalRet += retval;
	}
	return finalRet;
}


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
		printf("list[%d]= %s\n",i, list->key);
		list = list->next;
		i++;
	} while (head != list);
}


// generate random keys with length KEYLEN
char* gen_random(char* s, int size) {
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    int i;
    for (i = 0; i < size; ++i) {
        s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
    }
    s[size] = 0;
    return s;
}

// decide which list a key should be inserted.
// return the index of the list
int hash(const char* key){
	int len = strlen(key);
	// printf("len=%d\n", len);
	int sum = 0;
	int i;
	for(i = 0; i != len; i++){
		sum += key[i];
	}
	return sum % nLists;
}

void* threadfunc(void* ind){
	int index = *((int*) ind);
	free(ind);
	int length;
	int i;

	// insert each element 
	for(i = 0; i < iterations; ++i) {
		int j = hash(elements[index*(iterations) + i].key);
		// printf("j=%d\n", j);
		if(MUTEX){
			int ret = pthread_mutex_lock(&(mutex[j]));
			SortedList_insert(&list[j], &elements[index*(iterations) + i]);
			// printf("releasing mutex for insert, mutex=%d\n", mutex);
			pthread_mutex_unlock(&(mutex[j]));
		}
		else if(SPIN){
			while(__sync_lock_test_and_set(&(spinlock[j]),1));
			SortedList_insert(&list[j], &elements[index*(iterations) + i]);
			// SortedList_display(list);	
			__sync_lock_release(&(spinlock[j]));
		}else{
			SortedList_insert(&list[j], &elements[index*(iterations) + i]);
		}		
	}

	// get length
	if(MUTEX){
		// lock before entering "locking" process
		pthread_mutex_lock(&len_mutex);
		for(i = 0; i != nLists; i++)
			pthread_mutex_lock(&(mutex[i]));
		
		// critical section
		length = SortedList_length(list);

		for(i = 0; i != nLists; i++)
			pthread_mutex_unlock(&(mutex[i]));
		
		pthread_mutex_unlock(&len_mutex);
	}else if(SPIN){
		// lock before entering "locking" process
		while(__sync_lock_test_and_set(&len_spinlock,1));
		
		for(i = 0; i != nLists; i++)
			while(__sync_lock_test_and_set(&(spinlock[i]),1));
		
		length = SortedList_length(list);	

		for(i = 0; i != nLists; i++)
			__sync_lock_release(&(spinlock[i]));

		__sync_lock_release(&len_spinlock);
	}else{
		length = SortedList_length(list);
	}
	// printf("length = %d\n", length);
	// look up/ delete
	for(i = 0; i < iterations; ++i) {
		int j = hash(elements[index*(iterations) + i].key);
		SortedListElement_t* target;
		if(MUTEX){
			pthread_mutex_lock(&(mutex[j])); 	
			target = SortedList_lookup(&list[j], elements[index*(iterations) + i].key);
			if(target == NULL){
				printf("should never be here.\b");
				pthread_mutex_unlock(&(mutex[j]));
				continue;
			}
			SortedList_delete(target);
			pthread_mutex_unlock(&(mutex[j]));
		}else if(SPIN){
			while(__sync_lock_test_and_set(&(spinlock[j]),1));
			target =  SortedList_lookup(&list[j], elements[index*iterations + i].key);
			if(target == NULL){
				printf("should never be here.\b");
				__sync_lock_release(&(spinlock[j]));
				continue;
			}
			SortedList_delete(target);
			__sync_lock_release(&(spinlock[j]));
		}else{
			// printf("lookup\n");
			// printf("j=%d, key=%s\n", j,elements[index*(iterations) + i].key);
			target=  SortedList_lookup(&list[j], elements[index*iterations + i].key);	
			// printf("\tDONE loopup\n");
			if(target == NULL){
				printf("should never be here.\b");
				continue;
			}
			// printf("delete\n");
			SortedList_delete(target);
			// printf("\tDONE delete\n");
		}

	}

	// SortedList_display(list);
	// length = SortedList_length(list);
}

int main(int argc, char** argv){
	// for loop counter
	int i = 0;
  	// c holds return value of getopt_long
	int c;
	char* yield = NULL;
	struct timespec startTime, endTime;
	long long time_threads_create = 0;
	struct timespec starttime_threads_create, endtime_threads_create;
	  // Parse options
	while (1) {
	    int option_index = 0;
	    static struct option long_options[] = {
	// SUBCOMMAND
	   		{"iterations",       optional_argument,        0,  'i' },
        	{"threads",       optional_argument,        0,  't' },
        	{"yield",       optional_argument,        0,  'y' },
        	{"sync",		optional_argument,		0,	's'},
        	{"lists", 		optional_argument,		0,	'l'},
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
	    						// printf("y=insert\n");
	    						break;
	    					case 'd':
	    						opt_yield |= DELETE_YIELD;
	    						// printf("y=delte\n");
	    						break;
	    					case 's':
	    						opt_yield |= SEARCH_YIELD;
	    						// printf("y=search\n");
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
    						MUTEX = 1;
    						break;
    					case 's':
    						SPIN = 1;
    						break;
	    			}
	    		}
	    		break;
	    	case 'l':
	    		if(optarg != NULL)
	    			nLists = atoi(optarg);
	    			// printf("%d\n",nLists );
		    	break;
   		}

	}

	// initialize an empty list( with dummy)
	// list = malloc(sizeof(SortedList_t**))
	list = malloc(sizeof(SortedList_t)*nLists);
	// initialize mutexes 
	if(MUTEX){
    	pthread_mutex_init(&len_mutex, NULL);
		mutex = malloc(sizeof(pthread_mutex_t) * nLists);
		for(i = 0; i != nLists; i++)	
	    	pthread_mutex_init(&(mutex[i]), NULL);
    }
    // initialize spinlocks
    if(SPIN){
    	len_spinlock = 0;
    	spinlock = malloc(sizeof(int) * nLists);

		for(i = 0; i != nLists; i++)	
	    	spinlock[i] = 0;
    }
	for(i = 0; i != nLists; i++){
		list[i].key = NULL;
		list[i].prev = &list[i];
		list[i].next = &list[i];
	}
 // 	SortedList_t l;
 // 	list = &l;
 	
	// list->key = NULL;

	// create and initializes the required number
	nElements = nThreads * iterations;
	elements = malloc(sizeof(SortedListElement_t) * nElements);
	if(elements == NULL){
		fprintf(stderr, "Error in allocating space for elements\n");
	}
	keys= malloc(sizeof(char*) * nElements);
	if(elements == NULL){
		fprintf(stderr, "Error in allocating space for keys\n");
	}
	for(i = 0; i != nElements; i++){
		int size = rand() % 15;
		keys[i] = malloc(sizeof(char) * size);
		if(keys[i] == NULL){
			fprintf(stderr, "Error in allocating space for keys[%d]\n", i);
		}
		gen_random(keys[i],size);
	}

	for(i = 0 ; i != nElements; i++){
		elements[i].key = keys[i];
		elements[i].next = NULL;
		elements[i].prev = NULL;
	}

    //create threads
    pthread_t * thread_array = malloc(sizeof(pthread_t)* nThreads);
    
    // start time
    clock_gettime(CLOCK_MONOTONIC , &startTime);



    for(i = 0; i < nThreads; i++) {
    	clock_gettime(CLOCK_MONOTONIC , &starttime_threads_create);
    	int *arg = malloc(sizeof(*arg));
    	*arg = i;
    	// measure the time it takes to create thread
		int ret = pthread_create(&thread_array[i], NULL, threadfunc, (void *) arg);  //to create thread
		clock_gettime(CLOCK_MONOTONIC , &endtime_threads_create);
		time_threads_create += endtime_threads_create.tv_nsec - starttime_threads_create.tv_nsec;
		// printf("time_threads = %d\n", time_threads_create);
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


    // get avarage length of a sublist.

    // int avgLen = nElements / nLists;
    long num_ops = nThreads * (iterations * 2 + 1) ;

    printf("%d threads x (%d iterations x (ins + lookup/del) + len)  = %d operations\n", nThreads, iterations,  num_ops);
    if(counter != 0){
    	fprintf(stderr, "ERROR: final count = %d\n", counter);
    }


    long long total_time = endTime.tv_nsec - startTime.tv_nsec;
    printf("elapsed time: %d\n", total_time);
    printf("overhead time: %d\n", time_threads_create);
    long long avg = (total_time - time_threads_create)/ num_ops;
    printf("per operation: %d ns\n", avg);



  // printf("EXit with %d\n",exit_status );

    // free
    free(elements);
    for(i = 0; i != nElements; i++){
    	free(keys[i]);
    }
    free(keys);
    free(thread_array);
    free(list);
    free(spinlock);
    free(mutex);
	exit(0);


}
