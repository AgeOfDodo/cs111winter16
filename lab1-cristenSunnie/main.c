/* CS111 Winter 2016 Lab1  
   partners: Cristen Anderson <UID>
             Sunnie So  <704430286>
 */

//In Lab 1a, you'll warm up by implementing just the options --rdonly, --wronly, --command, and --verbose.
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>

/////// TO DO ///////////////////////////////////////////////////////////////
// done - CHECK OPEN ERROR AND EXIT LOOP DON'T STORE FILE DESCRIPTOR IN ARRAY IF ITS -1
// done - FREE!!!!! memory from malloc
// command execution 
//
//
//
//
//
//
//
//
//
//////////////////////////////////////////////////////////////////////////////////////

int checkOpenError(int fd) {
	if (fd == -1) {
		fprintf(stderr, "open returned unsuccessfully\n");
    return -1;
  }
  return 0;
}

int passChecks(char* str, int index, int num_args) {
  int i = 0;
  // checks if is a digit
  while (str != NULL && *(str+i) != '\0') {
    if (!isdigit(*(str+i))) {
      fprintf(stderr, "Incorrect usage of --command. Requires integer argument.\n");
      return 0;
    }
    i++;
  }
  // checks if is within number of arguments
  if (index >= num_args) {
    fprintf(stderr, "Invalid number of arguments for --command\n");
    return 0;
  }
  return 1;
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

    // Parse options
   while (1) {
        int option_index = 0;
        static struct option long_options[] = {
        	// { "name", has_arg, *flag, val }
            {"rdonly",     required_argument, 0,  'r' },
            {"wronly",  required_argument,  0,  'w' },
            {"command",  required_argument, 0,  'c' },
            {"verbose", no_argument, 0, 'v' },
        };

        // get the next option
       c = getopt_long(argc, argv, "",
                 long_options, &option_index);
       
       // break when there are no further options to parse
       if (c == -1)
            break;

       switch (c) {
       // read only
       case 'r':
            printf("option r with value '%s'\n", optarg);
            int read_fd = open(optarg, O_RDONLY);
            if(checkOpenError(read_fd) == -1) 
              continue;
            if (fd_array_cur == fd_array_size) {
            	fd_array_size *= 2;
            	fd_array = (int*)realloc((void*)fd_array, fd_array_size); 
            }
            fd_array[fd_array_cur] = read_fd;
            fd_array_cur++;
            printf("read_fd = %d\n", read_fd);
            break;
       // write only
       case 'w':
            printf("option w with value '%s'\n", optarg);
            int write_fd = open(optarg, O_WRONLY);  
            if(checkOpenError(read_fd) == -1) 
              continue;
            if (fd_array_cur == fd_array_size) {
            	fd_array_size *= 2;
            	fd_array = (int*)realloc((void*)fd_array, fd_array_size); 
            }
            fd_array[fd_array_cur] = write_fd;
            fd_array_cur++;
            printf("write_fd = %d\n", write_fd);
            break;
        // command. 
       case 'c': 
            printf("option c with value '%s'\n", optarg);
            //format: --command i o e cmd args_array
            int i,o,e;            //input, output, error  
            char* cmd;   //command name
            size_t args_array_size = 2; 
            char** args_array = malloc(args_array_size*sizeof(char*)); //command argument(s)
            int args_array_cur = 0;    //current index for the above array


            int index = optind; //current element from argv.

            /////////////////////////// 
            /**SET UP FD & ARGUMENTS**/
            ///////////////////////////
            //store the file descripter numbers and check for errors

            if (!passChecks(optarg, index, argc)) { break; }
            i = atoi(optarg);
            
            if (!passChecks(argv[index], index, argc)) { break; }
            o = atoi(argv[index]); index++;
            
            if (!passChecks(argv[index], index, argc)) { break; }
            e = atoi(argv[index]); index++;

            if (index >= argc) {
              fprintf(stderr, "Invalid number of arguments for --command\n");
              break;
            }
            cmd = argv[index]; index++;

            //store arguments of the command into an array of char**
            while(index < argc){
              //break the loop if the index reaches the next "--"option
              if(argv[index][0] == '-' && argv[index][1] == '-')
                break;
              //now this must be an argument for the command. Store it into args array
              //realloc: same mechanics as fd_array
              if(args_array_cur == args_array_size){
                args_array_size *= 2;
                args_array = (char**)realloc((void*)args_array, args_array_size); 
              }
              args_array[args_array_cur] = argv[index];
              printf("args_array[%d] = %s\n", args_array_cur, argv[index]);
              args_array_cur++;
              index++;
            }
            //set optind to the next in argv (next option)
            optind = index;
            printf("--command %d %d %d %s\n", i,o,e,cmd);

            ///////////////////////// 
            /**EXECUTE THE COMMAND**/
            /////////////////////////
            //TODO: fork() ==> let child process execute the command.
            // blah. 


            free(args_array);
        
            break;

       // verbose
       case 'v':
            printf("option v\n");
            break;
       // ? returns when doesn't recognize option character
       case '?':
            break;

       default:
            printf("?? getopt returned character code 0%o ??\n", c);
        }
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
