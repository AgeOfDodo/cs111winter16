/* CS111 Winter 2016 Lab1a
See README for further information
 */

#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/wait.h>
#include <sys/types.h>

// Check if a file descriptor is valid
int validFd(int fd, int fd_array_cur){
	if( fd >= fd_array_cur){	
  		fprintf(stderr, "Error: Invalid use of file descriptor %d before initiation.\n", fd);
  		return 0;
  	}
  	return 1;
}

// Check if the open system call had an error
int checkOpenError(int fd) {
	if (fd == -1) {
		fprintf(stderr, "Error: open returned unsuccessfully\n");
    return -1;
  }
  return 0;
}

// Checks for --command arguments
int passChecks(char* str, int index, int num_args) {
  int i = 0;
  // checks if is a digit
  while (str != NULL && *(str+i) != '\0') {
    if (!isdigit(*(str+i))) {
      fprintf(stderr, "Error: Incorrect usage of --command. Requires integer argument.\n");
      return 0;
    }
    i++;
  }
  // checks if is within number of arguments
  if (index >= num_args) {
    fprintf(stderr, "Error: Invalid number of arguments for --command\n");
    return 0;
  }
  return 1;
}

// Stores all arguments for a command in args_array, until the next "--" flag
// Doesn't include optarg which is accepted automatically.
// Keeps the current index updated and returns the updated argv index (optind)
int findArgs(char** args_array, size_t args_array_size, 
              int index, int* args_current,
              int argc, char** argv) {
  
  int args_array_cur = *args_current;
  //store arguments of the command into an array of char**
  while(index < argc){
    //break the loop if the index reaches the next "--"option
    if(argv[index][0] == '-' && argv[index][1] == '-') {
      break;
    }
      
    //now this must be an argument for the command. Store it into args array
    //realloc: same mechanics as fd_array
    if(args_array_cur == args_array_size){
      args_array_size *= 2;
      args_array = (char**)realloc((void*)args_array, args_array_size*sizeof(char*)); 
    }
    args_array[args_array_cur] = argv[index];
    args_array_cur++;
    index++;
  }
  *args_current = args_array_cur;
  return index;
}

int main(int argc, char **argv) {
  // c holds return value of getopt_long
  int c;

  // j is an iterator for for loops
  int j;

  // will be updated to 1 if any calls to open fail
  int exit_status = 0;

  // Declare array to hold file descriptors
  size_t fd_array_size = 2;
  int fd_array_cur = 0;
  int * fd_array = malloc(fd_array_size*sizeof(int));

  // open flag
  int oflag;

  // Verbose can be on or off, automatically set to off
  int verbose = 0;

  // Parse options
  while (1) {
    int option_index = 0;
    static struct option long_options[] = {
    	// { "name",      has_arg,         *flag, val }
        {"rdonly",      required_argument,  0,  'r' },
        {"wronly",      required_argument,  0,  'w' },
        {"command",     required_argument,  0,  'c' },
        {"verbose",     no_argument,        0,  'v' },
        {"wait",        no_argument,        0,  'z'}
    };

    // get the next option
    c = getopt_long(argc, argv, "", long_options, &option_index);
   
    // break when there are no further options to parse
    if (c == -1)
      break;

    // args_array will store flag and its arguments
    size_t args_array_size = 2; 
    char** args_array = malloc(args_array_size*sizeof(char*)); //command argument(s)
    int args_array_cur = 0;    //current index for the above array  

    switch (c) {
    
    case 'r': // read only 
    case 'w': // write only
   		// assign oflag
      if (c == 'r') 	oflag = O_RDONLY;
   		else 			oflag = O_WRONLY;
      
      // find all arguments for the current flag
      optind = findArgs(args_array, args_array_size, optind, &args_array_cur,
                        argc, argv);

      // print command if verbose is enabled
      if (verbose) {
        char * flags;
        if (c == 'r') flags = "--rdonly";
        else flags = "--wronly";
        printf("%s ", flags);
        printf("%s ", optarg);
        for (j = 0; j < args_array_cur; j++) {
          printf("%s ", args_array[j]);
        }
        printf("\n");
      }
      
      // open file into read write file descriptor
      int rw_fd = open(optarg, oflag, 777);
      if(checkOpenError(rw_fd) == -1) {
        exit_status = 1;
        continue;
      }
      
      // save file descriptor to array
      if (fd_array_cur == fd_array_size) {
      	fd_array_size *= 2;
      	fd_array = (int*)realloc((void*)fd_array, fd_array_size); 
      }
      fd_array[fd_array_cur] = rw_fd;
      fd_array_cur++;
      break;   
      
      case 'c': { // command (format: --command i o e cmd args_array)
      int i, o, e; // stdin, stdout, stderr

      //store the file descripter numbers and check for errors
      if (!passChecks(optarg, optind, argc)) { break; }
      i = atoi(optarg);
      
      if (!passChecks(argv[optind], optind, argc)) { break; }
      o = atoi(argv[optind]); optind++;
      
      if (!passChecks(argv[optind], optind, argc)) { break; }
      e = atoi(argv[optind]); optind++;

      // check if there is the proper number of arguments
      if (optind >= argc) {
        fprintf(stderr, "Error: Invalid number of arguments for --command\n");
        break;
      }

      // save command into args array
      args_array[0] = argv[optind]; optind++;
      args_array_cur++;

      // find arguments for command
      optind = findArgs(args_array, args_array_size, optind, &args_array_cur,
                        argc, argv);
      
      //append NULL to args_array (necessary for execvp())
      if(args_array_cur == args_array_size){
          args_array_size++;
          args_array = (char**)realloc((void*)args_array, args_array_size*sizeof(char*)); 
      }
      args_array[args_array_cur] = NULL;
      args_array_cur++;

      // print if verbose
      if (verbose == 1) {
        printf("--command %d %d %d ", i,o,e);
        for (j = 0; j < args_array_cur-1; j++) {
          printf("%s ", args_array[j]);
        }
        printf("\n");
      }

      //check if i,o,e fd are valid 
      if(!(validFd(i,fd_array_cur) && validFd(o,fd_array_cur) && validFd(e,fd_array_cur)))  
        continue;

      // execute command
      pid_t pid = fork();
      if(pid == 0){   //child process
        //redirect stdin to i, stdout to o, stderr to e
        dup2(fd_array[i], 0);
        dup2(fd_array[o], 1);
        dup2(fd_array[e], 2);

        execvp(args_array[0], args_array);
        //return to main program if execvp fails
        fprintf(stderr, "Error: Unknown command '%s'\n", args_array[0]);
        exit(255);  
      }
      break;
    }
     
    case 'v': // verbose
      verbose = 1;
      break;
      
    case 'z': {
      int status;
      //wait any child process to finish. 0 is for blocking.
      pid_t returnedPid = waitpid(WAIT_ANY, &status, 0);
      //WEXITSTATUS returns the exit status of the child.
      int waitStatus = WEXITSTATUS(status);
      printf("%d ", waitStatus);
      if (waitStatus > exit_status) {
        exit_status = waitStatus;
      }
      for (j = 0; j < args_array_cur-1; j++) {
        printf("%s ", args_array[j]);
      }
      printf("\n");
      break;
    }
     
    case '?': // ? returns when doesn't recognize option character
      break;

    default:
        fprintf(stderr, "Error: ?? getopt returned character code 0%o ??\n", c);
    }
    // Free arguments array for next command
    free(args_array);
  }

  // Prints out extra options that weren't parsed
 // if (optind < argc) {
 //      printf("non-option ARGV-elements: ");
 //      while (optind < argc)
 //          printf("%s ", argv[optind++]);
 //      printf("\n");
 //  }


  // Close all used file descriptors
  fd_array_cur--;
  while (fd_array_cur >= 0) {
  	close(fd_array[fd_array_cur]);
  	fd_array_cur--;
  }

  // Free file descriptor array
  free(fd_array);

  // Exit with previously set status
  exit(exit_status);
}
