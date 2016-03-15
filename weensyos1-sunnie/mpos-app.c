#include "mpos-app.h"
#include "lib.h"

/*****************************************************************************
 * mpos-app
 *
 *   This application simply starts a child process and then waits for it
 *   to exit.  Both processes print messages to the screen.
 *
 *****************************************************************************/

void run_child(void);
void exer6_test(void);
void
start(void)
{
	volatile int checker = 0; /* This variable checks that you correctly
				     gave the child process a new stack. */
	pid_t p;
	int status;
	//tests for exercise 6. threads. 
	//Change if(0) to if(1) to show test results
	if(0){
		app_printf("About to start a new thread...\n");		
		p = sys_newthread(&exer6_test);
		// thread should automatically call exer6_test, so
		// no need to do anything with p == 0.
		// let the parent wait on the thread
		if(p != 0){
			sys_wait(p);
		}
	}
	app_printf("About to start a new process...\n");

	p = sys_fork();
	if (p == 0)
		run_child();
	else if (p > 0) {
		app_printf("Main process %d!\n", sys_getpid());
		// do {
		status = sys_wait(p);
			//EDIT
			// app_printf("W");
			//

		// } while (status == WAIT_TRYAGAIN);
		app_printf("Child %d exited with status %d!\n", p, status);

		// Check whether the child process corrupted our stack.
		// (This check doesn't find all errors, but it helps.)
		if (checker != 0) {
			app_printf("Error: stack collision!\n");
			sys_exit(1);
		} else
			sys_exit(0);

	} else {
		app_printf("Error!\n");
		sys_exit(1);
	}
}

void
run_child(void)
{
	int i;
	volatile int checker = 1; /* This variable checks that you correctly
				     gave this process a new stack.
				     If the parent's 'checker' changed value
				     after the child ran, there's a problem! */

	app_printf("Child process %d!\n", sys_getpid());

	// Yield a couple times to help people test Exercise 3
	for (i = 0; i < 20; i++)
		sys_yield();

	sys_exit(1000);
}

void
exer6_test(void){
	app_printf("Thread %d!\n", sys_getpid());
	//some random exit status.
	sys_exit(30);
}