#include "mpos-app.h"
#include "lib.h"

/*****************************************************************************
 * mpos-app2
 *
 *   This application as 1024 new processes and then waits for them to
 *   exit.  All processes print messages to the screen.
 *
 *****************************************************************************/

volatile int counter;

void run_child(void);

void
start(void)
{
	pid_t p;
	int status;

	counter = 0;

	while (counter < 1025) {
		int n_started = 0;

		// Start as many processes as possible, until we fail to start
		// a process or we have started 1025 processes total.
		while (counter + n_started < 1025) {
			p = sys_fork();
			if (p == 0)
				run_child();
			else if (p > 0)
				n_started++;
			else
				break;
		}

		// If we could not start any new processes, give up!
		if (n_started == 0)
			break;

		// We started at least one process, but then could not start
		// any more.
		// That means we ran out of room to start processes.
		// Retrieve old processes' exit status with sys_wait(),
		// to make room for new processes.
		for (p = 2; p < NPROCS; p++)
			(void) sys_wait(p);
	}

	sys_exit(0);
}

void
run_child(void)
{
	int input_counter = counter;

	counter++;		/* Note that all "processes" share an address
				   space, so this change to 'counter' will be
				   visible to all processes. */

	// exercise 7: kill all odd numbered processes
	// Loop begin at i = 3 because we don't use proc_array[0], 
	// and proc_array[1] is the main process. The iterater increments
	// by two to ensure it only kills odd numbered processes.
	pid_t curr_pid = sys_getpid();
	if(curr_pid % 2 == 0){
		pid_t i;
		for(i = 3; i < NPROCS; i=i+2){
			sys_kill(i);
		}
		// I put the below output inside the if brackets because some 
		// odd numbered process (i.e.pid 3) may reach to this line before 
		// it is killed by the even numbered process.
		app_printf("Process %d lives, counter %d!\n",
			   curr_pid, input_counter);

	}
	sys_exit(input_counter);
}
