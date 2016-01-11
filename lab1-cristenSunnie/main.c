/* CS111 Winter 2016 Lab1  
See README for further information
 */

//In Lab 1a, you'll warm up by implementing just the options --rdonly, --wronly, --command, and --verbose.
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/wait.h>
#include <sys/types.h>

/////// TO DO ///////////////////////////////////////////////////////////////
// implement verbose
// update README
// update test.sh with test cases
// use make check
// exit status = sum of exit statuses of subcommands that ran and waited for
// check if multiple arguments for read/write
// use argv for verbose? just print out from a beginning of option index to the end.
// 
//
//////////////////////////////////////////////////////////////////////////////////////
int validFd(int fd, int fd_array_cur){
	if( fd >= fd_array_cur){	
  		fprintf(stderr, "Error: Invalid use of file descriptor %d before initiation.\n", fd);
  		return 0;
  	}
  	return 1;
}

int checkOpenError(int fd) {
	if (fd == -1) {
		fprintf(stderr, "Error: open returned unsuccessfully\n");
    return -1;
  }
  return 0;
}

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

int
main(int argc, char **argv)
{
    // c holds return value of getopt_long
    int c;

    // Declare array to hold file descriptors
    size_t fd_array_size = 2;
    int fd_array_cur = 0;
    int * fd_array = malloc(fd_array_size*sizeof(int));
    int oflag;
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
        };

        // get the next option
       c = getopt_long(argc, argv, "",
                 long_options, &option_index);
       
       // break when there are no further options to parse
       if (c == -1)
            break;

       // args_array will store flag and its arguments
      size_t args_array_size = 2; 
      char** args_array = malloc(args_array_size*sizeof(char*)); //command argument(s)
      int args_array_cur = 0;    //current index for the above array  
      int i,o,e; 
       switch (c) {
       // read only
       case 'r':
       // write only
       case 'w':   		
       		if (c == 'r') 	oflag = O_RDONLY;
       		else 			oflag = O_WRONLY;
          optind = findArgs(args_array, args_array_size, optind, &args_array_cur,
                            argc, argv);

          if (verbose) {
            char * flags;
            if (c == 'r') { flags = "--rdonly"; } 
            else { flags = "--wronly"; }
            printf("%s ", flags);
            printf("%s ", optarg);
            int j;
            for (j = 0; j < args_array_cur; j++) {
              printf("%s ", args_array[j]);
            }
            printf("\n");
          }
          
          // read write file descriptor
          int rw_fd = open(optarg, oflag, 777);
          if(checkOpenError(rw_fd) == -1) 
            continue;
          if (fd_array_cur == fd_array_size) {
          	fd_array_size *= 2;
          	fd_array = (int*)realloc((void*)fd_array, fd_array_size); 
          }
          fd_array[fd_array_cur] = rw_fd;
          fd_array_cur++;
          break;
            
        // command. 
       case 'c': 
        //format: --command i o e cmd args_array
                   //input, output, error  

        /////////////////////////// 
        /**SET UP FD & ARGUMENTS**/
        ///////////////////////////
        //store the file descripter numbers and check for errors

        if (!passChecks(optarg, optind, argc)) { break; }
        i = atoi(optarg);
        
        if (!passChecks(argv[optind], optind, argc)) { break; }
        o = atoi(argv[optind]); optind++;
        
        if (!passChecks(argv[optind], optind, argc)) { break; }
        e = atoi(argv[optind]); optind++;

        if (optind >= argc) {
          fprintf(stderr, "Error: Invalid number of arguments for --command\n");
          break;
        }
        args_array[0] = argv[optind]; optind++;
        args_array_cur++;

        optind = findArgs(args_array, args_array_size, optind, &args_array_cur,
                          argc, argv);
        
        //append NULL to args_array (necessary for execvp())
        if(args_array_cur == args_array_size){
            args_array_size++;
            args_array = (char**)realloc((void*)args_array, args_array_size*sizeof(char*)); 
        }
        args_array[args_array_cur] = NULL;
        args_array_cur++;

        if (verbose == 1) {
          printf("--command %d %d %d ", i,o,e);
          int j;
          for (j = 0; j < args_array_cur-1; j++) {
            printf("%s ", args_array[j]);
          }
          printf("\n");
        }

        ///////////////////////// 
        /**EXECUTE THE COMMAND**/
        /////////////////////////

        //check if i,o,e fd are valid 
        if(!(validFd(i,fd_array_cur) && validFd(o,fd_array_cur) && validFd(e,fd_array_cur)))	continue;


        pid_t pid = fork();
        int status;
        if(pid == 0){   //child process
          printf("Enter child process\n");
          //redirect stdin to i, stdout to o, stderr to e
          dup2(fd_array[i], 0);
          dup2(fd_array[o], 1);
          dup2(fd_array[e], 2);

          execvp(args_array[0], args_array);
          //return to main program if execvp fails
          fprintf(stderr, "Error: Unknown command '%s'\n", args_array[0]);
          exit(255);  
        }else{  //parent process
          printf("Enter parent process\n");
          //wait any child process to finish. 0 is for blocking.
          pid_t returnedPid = waitpid(WAIT_ANY, &status, 0);
          //WEXITSTATUS returns the exit status of the child.
          printf("Child exit code: %d\n", WEXITSTATUS(status));
          
        }
        break;

       // verbose
       case 'v':
            printf("--verbose\n");
            verbose = 1;
            break;
       // ? returns when doesn't recognize option character
       case '?':
            break;

       default:
            fprintf(stderr, "?? getopt returned character code 0%o ??\n", c);
        }
        free(args_array);
    }

    // Prints out extra options that weren't parsed
   if (optind < argc) {
        printf("non-option ARGV-elements: ");
        while (optind < argc)
            printf("%s ", argv[optind++]);
        printf("\n");
    }


    // Close all used file descriptors
    fd_array_cur--;
    while (fd_array_cur >= 0) {
    	close(fd_array[fd_array_cur]);
    	fd_array_cur--;
    }
    free(fd_array);
   exit(EXIT_SUCCESS);
}
