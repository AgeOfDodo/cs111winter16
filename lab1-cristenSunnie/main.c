//lab1 Cristen, Sunnie CS111 Winter 2016

//In Lab 1a, you'll warm up by implementing just the options --rdonly, --wronly, --command, and --verbose.
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/stat.h>
#include <fcntl.h>

/////// TO DO ///////////////////////////////////////////////////////////////
// CHECK OPEN ERROR AND EXIT LOOP DON'T STORE FILE DESCRIPTOR IN ARRAY IF ITS -1
// FREE!!!!! memory from malloc
//
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

void checkOpenError(int fd) {
	if (fd == -1) {
		fprintf(stderr, "open returned unsuccessfully\n");
	}
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
            checkOpenError(read_fd);
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
            checkOpenError(write_fd);
            if (fd_array_cur == fd_array_size) {
            	fd_array_size *= 2;
            	fd_array = (int*)realloc((void*)fd_array, fd_array_size); 
            }
            fd_array[fd_array_cur] = write_fd;
            fd_array_cur++;
            printf("write_fd = %d\n", write_fd);
            break;
        // command
       case 'c':
            printf("option c with value '%s'\n", optarg);
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

   exit(EXIT_SUCCESS);
}
