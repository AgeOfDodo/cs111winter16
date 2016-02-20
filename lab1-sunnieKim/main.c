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
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <math.h>

#define MAX(a,b) (((a)>(b))?(a):(b))


int strIsNum(char* str){
  int j;
  for(j = 0; str != NULL && *(str+j) != '\0'; j++) {
    if (!isdigit(*(str+j))) {
      return 0;
    }
  }
  return 1;
}
// [command] Check if a file descriptor is valid
int validFd(int fd, int fd_array_cur, int** fd_array){
	if( fd >= fd_array_cur){	
  	fprintf(stderr, "Error: Invalid use of file descriptor %d before initiation.\n", fd);
  	return 0;
  }
  if((*fd_array)[fd] == -1){
    fprintf(stderr, "Error: Invalid access to file descriptor %d.\n", fd);
  	return 0;
  }
  return 1;
}

// [read write] Check if the open system call had an error
int checkOpenError(int fd) {
	if (fd == -1) {
		fprintf(stderr, "Error: open returned unsuccessfully\n");
    return -1;
  }
  return 0;
}

// [command] Checks for --command arguments
int passChecks(char* str, int index, int num_args) {
  int i = 0;
  // checks if is a digit
 
  if(!strIsNum(str)){
    fprintf(stderr, "Error: Incorrect usage of --command. Requires integer argument.\n");
    return 0;  
  }
  // checks if is within number of arguments
  if (index >= num_args) {
    fprintf(stderr, "Error: Invalid number of arguments for --command\n");
    return 0;
  }
  return 1;
}


//[command]
// Stores all arguments for a command in args_array, until the next "--" flag
// Doesn't include optarg which is accepted automatically.
// Keeps the current index updated and returns the updated argv index (optind)
int findArgs(char*** args_array, size_t args_array_size, 
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
      *args_array = (char**)realloc(*args_array, args_array_size*sizeof(char*)); 
    }
    (*args_array)[args_array_cur] = argv[index];
    args_array_cur++;
    index++;
  }
  *args_current = args_array_cur;
  return index;
}


struct waitInfo{
  pid_t childPid;
  int begin;
  int end;
};


void catch_handler(int sig, siginfo_t *s, void *arg){
  fprintf(stderr, "%d caught\n", sig);
  exit(sig); 
}
void pause_handler(int sig, siginfo_t *s, void *arg){
  fprintf(stderr, "pause\n");
}
int main(int argc, char **argv) {

  int profile = 0;
  int who = RUSAGE_SELF;



  struct rusage usage_start;
  struct rusage usage_end;
  struct rusage children;    

  //for --wait, collect the time usage from all waited child processes
  long long childrenTimeu = 0;
  long long childrenTimes = 0;
  //the beginning of the while loop
  long long start_u;
  long long start_s;
  //the beginning of the whole program
  long long begin_u;
  long long begin_s;

  int ret;

  //[profile]
  //get the beginning usage 
  getrusage(RUSAGE_SELF, &usage_start);
  begin_u = (long long )usage_start.ru_utime.tv_sec*pow(10, 6) + (long long )usage_start.ru_utime.tv_usec;
  begin_s = (long long )usage_start.ru_stime.tv_sec*pow(10, 6) + (long long )usage_start.ru_stime.tv_usec;
  //printf("[profile] begin = %lld\n", beginu );
  
  // ret = getrusage(who, &usage);

  // c holds return value of getopt_long
  int c;



  // j is an iterator for for loops
  int j;

  // N is signal variable.
  int N;

  // status for child process
  int status;
  // will be updated to 1 if any calls to open fail
  int exit_status = 0;

  // Declare array to hold file descriptors
  size_t fd_array_size = 2;
  int fd_array_cur = 0;
  int * fd_array = malloc(fd_array_size*sizeof(int));
  // Declare a fd_isPipe.   
  int * fd_isPipe = malloc(fd_array_size*sizeof(int));/*fd_isPipe[fd] is 1 if fd is  pipe descripter.
                                                        fd_isPipe[fd] is  0 if fd is regular file descripters.*/

  // abort flag
  int ignoreAbort = 0;

  // open flag
  int oflag = 0;
  char oflag_str[1000];
  strcpy(oflag_str, "");
  // [profile]
  // if the oflag is not 0, it means that we are currently at a potential beginning of file flag serie
  // (ex. --append --create --rdonly . we only wanna get get usage right before --append and within the --rdonly clause) 
  
  // Verbose can be on or off, automatically set to off
  int verbose = 0;

  // wait record structure 
  size_t wait_info_size = 2;
  int wait_info_cur = 0;
  struct waitInfo* wait_info = malloc(wait_info_size * sizeof *wait_info);

  // Signal handling.
  struct sigaction sa;

  // Parse options
  while (1) {

    if(profile && (oflag==0)){
      getrusage(RUSAGE_SELF,&usage_start);

      start_u = (long long ) (usage_start.ru_utime.tv_sec*pow(10, 6) +  usage_start.ru_utime.tv_usec);
      start_s = (long long ) (usage_start.ru_stime.tv_sec*pow(10, 6) +  usage_start.ru_stime.tv_usec);        

      // printf("[profile] begins = %lld\n", (long long) start_s);
    }
    int option_index = 0;
    static struct option long_options[] = {
// SUBCOMMAND
    	  {"command",     required_argument,  0,  'c' },
        {"wait",        no_argument,        0,  'z' },
// FILE FLAGS
        {"append",      no_argument,        0,  6 },
        {"cloexec",     no_argument,        0,  7 },
        {"creat",       no_argument,        0,  8 },
        {"directory",   no_argument,        0,  9 },
        {"dsync",       no_argument,        0,  10 },
        {"excl",        no_argument,        0,  11 },
        {"nofollow",    no_argument,        0,  12 },
        {"nonblock",    no_argument,        0,  13 },
        {"rsync",       no_argument,        0,  14 },
        {"sync",        no_argument,        0,  15 },
        {"trunc",       no_argument,        0,  16 },
// FILE OPENING
        {"rdonly",      required_argument,  0,  17 },
        {"wronly",      required_argument,  0,  18 },
        {"rdwr",        required_argument,  0,  19 },
        {"pipe",        no_argument,        0,  20 },
 
// MISCELLANEOUS  
        {"close",      required_argument,   0,  28 },
        {"verbose",     no_argument,        0,  21 },
        {"profile",     no_argument,        0,  22 },
        {"abort",       no_argument,        0,  23 },
        {"catch",       required_argument,  0,  24 },
        {"ignore",      required_argument,  0,  25 },
        {"default",     required_argument,  0,  26 },
        {"pause",       no_argument,        0,  27 },
        {0,0,0,0}
        
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
    
//command
    case 'c': { // command (format: --command i o e cmd args_array)
      //[profile]
      //On piazza, it says that we only time whatever happens in the parent process 
      if(profile && (oflag == 0)){ 
        getrusage(RUSAGE_SELF, &usage_start);
      }
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
        exit_status = MAX(1,exit_status);
        continue;
      }
      //[--wait] Record the location of the command substring in argv for --wait to output later
      int cmd_begin = optind;
      int cmd_end;

      // save command into args array
      args_array[0] = argv[optind]; optind++;
      args_array_cur++;

      // find arguments for command
      optind = findArgs(&args_array, args_array_size, optind, &args_array_cur,
                        argc, argv);
      cmd_end = optind;

      if(args_array_cur == args_array_size){
          args_array_size*=2;
          args_array = (char**)realloc((void*)args_array, args_array_size*sizeof(char*)); 
      }
      //append NULL to args_array (necessary for execvp())
      args_array[args_array_cur] = NULL;
      args_array_cur++;

      // [verbose]
      if (verbose == 1) {
        printf("--command %d %d %d ", i,o,e);
        for (j = 0; j < args_array_cur-1; j++) {
          printf("%s ", args_array[j]);
        }
        printf("\n");
      }

      //check if i,o,e fd are valid 
      if(!(validFd(i,fd_array_cur, &fd_array) && validFd(o,fd_array_cur, &fd_array) && validFd(e,fd_array_cur, &fd_array))){  
        exit_status = MAX(exit_status, 1);
        continue;
      }

      args_array[args_array_cur] = NULL;
      args_array_cur++;

      
      // execute command      
      pid_t pid = fork();
      if(pid == 0){   //child process
      
        /* StackOverflow:If you use dup() or dup2() to duplicate one end of a pipe 
           to standard input or standard output, you need to close() both ends of 
           the original pipe.
         */
        
        //redirect stdin to i, stdout to o, stderr to e
        dup2(fd_array[i], 0);
        dup2(fd_array[o], 1);
        dup2(fd_array[e], 2);
        //if use a pipe, close the other end of the pipe
        if(fd_isPipe[i] != 0)
          close(fd_array[i+1]);
        if(fd_isPipe[o] != 0)
          close(fd_array[o-1]);
        if(fd_isPipe[e] != 0)
          close(fd_array[e-1]);

        close(fd_array[i]);
        close(fd_array[o]);
        close(fd_array[e]);
        execvp(args_array[0], args_array);
        //return to main program if execvp fails
        fprintf(stderr, "Error: Unknown command '%s'\n", args_array[0]);

        exit(255);  
       }
       if(fd_isPipe[i] != 0){
          close(fd_array[i]);
          fd_array[i] = -1;
       }

        if(fd_isPipe[o] != 0){
          close(fd_array[o]);
          fd_array[i] = -1;
        }
        if(fd_isPipe[e] != 0){
          close(fd_array[e]);
          fd_array[i] = -1;
        }
      // close(fd_array[i]);
      // close(fd_array[o]);
      // close(fd_array[e]);
      //record command in wait_output_chain for --wait
      if(wait_info_cur == wait_info_size){
          wait_info_size *=2;
          wait_info = realloc((void*)wait_info, wait_info_size*sizeof *wait_info); 
      }
      wait_info[wait_info_cur].childPid = pid;
      wait_info[wait_info_cur].begin = cmd_begin;
      wait_info[wait_info_cur++].end = cmd_end;

      //[profile]
      // //On piazza, it says that we only time whatever happens in the parent process     
      if(profile){
        getrusage(RUSAGE_SELF, &usage_end);
        // long long end= (long long)usage.ru_utime.tv_sec*pow(10, 6) + (long long)usage.u_utime.tv_usec;
        long long endu = (long long ) (usage_end.ru_utime.tv_sec*pow(10, 6) +  usage_end.ru_utime.tv_usec);
        long long ends = (long long ) (usage_end.ru_stime.tv_sec*pow(10, 6) +  usage_end.ru_stime.tv_usec);
        // printf("endu = %lld\n", ends );
        printf("[profile] user CPU time => %lld us\t system CPU time => %lld us\n",
        endu - start_u, ends - start_s);
      }
      break;
    }
//wait
    case 'z': {
      if (verbose){
        printf("--wait\n");
      }


      //wait any child process to finish. 0 is for blocking.
      pid_t returnedPid;
      int j1;
      while((returnedPid = waitpid(0, &status, 0) )!= -1){
       //WEXITSTATUS returns the exit status of the child.
        int waitStatus = WEXITSTATUS(status);
        printf("%d ", waitStatus);

        //the main program needs to return the with biggest value of exit status.
        exit_status = MAX(exit_status, waitStatus);
        //find the corresponding wait_info
        for(j = 0 ; j != wait_info_cur; j++){
          if(returnedPid == (wait_info[j]).childPid)
            break;
        }
        //print out the corresponding command lines from wait_info data structure.
        int j1= j;
        for (j = wait_info[j1].begin; j != wait_info[j1].end; j++) {
          printf("%s ", argv[j]);
        }
        printf("\n");


      }
        //[profile] 
        // on Piazza, Zhaoxing said:  you time 
        // (1) whatever happens in the parent process and 
        // (2) sum of all child processes you have waited or report each child process separately.
      if(profile){
        getrusage(RUSAGE_CHILDREN, &children);
        // long long end= (long long)usage.ru_utime.tv_sec*pow(10, 6) + (long long)usage.u_utime.tv_usec;
        childrenTimeu = (long long ) (children.ru_utime.tv_sec*pow(10, 6) +  children.ru_utime.tv_usec);
        childrenTimes = (long long ) (children.ru_stime.tv_sec*pow(10, 6) +  children.ru_stime.tv_usec);
        // printf("endu = %lld\n", ends );
        printf("[profile] Child Processes Total...\n\t  user CPU time => %lld us\t system CPU time => %lld us\n",
        childrenTimeu , childrenTimes );
      }
      break;
    }
     
//append
    case 6:
      oflag |= O_APPEND;  strcat(oflag_str,"--append ");
    break;
//cloexev
    case 7:
      oflag |= O_CLOEXEC;  strcat(oflag_str,"--cloexev ");
    break;
//creat
    case 8:
      oflag |= O_CREAT;  strcat(oflag_str,"--creat ");
    break;
//directory
    case 9:
      oflag |= O_DIRECTORY;  strcat(oflag_str,"--directory ");
    break;
//dsync
    case 10:
      oflag |= O_DSYNC;  strcat(oflag_str,"--dsync ");
    break;
//excl
    case 11:
      oflag |= O_EXCL;  strcat(oflag_str,"--excl ");
    break;
//nofollow
    case 12:
      oflag |= O_NOFOLLOW;  strcat(oflag_str,"--nofollow ");
    break;
//nonblock
    case 13:
      oflag |= O_NONBLOCK;  strcat(oflag_str,"--nonblock ");
    break;
//rsync
    case 14:
      oflag |= O_RSYNC;  strcat(oflag_str,"--rsync ");
    break;
//sync
    case 15:
      oflag |= O_SYNC;  strcat(oflag_str,"--sync ");
    break;
//trunc
    case 16:
      oflag |= O_TRUNC;  strcat(oflag_str,"--trunc ");
    break;
//read only
    case 17: 
//write only
    case 18:
//read and write 
    case 19: 
      // assign oflag
      if (c == 17)      oflag = O_RDONLY | oflag;
      else if( c== 18)  oflag = O_WRONLY | oflag;
      else              oflag = O_RDWR | oflag;
      // find all arguments for the current flag
      optind = findArgs(&args_array, args_array_size, optind, &args_array_cur,
                        argc, argv);

      // print command if verbose is enabled
      if (verbose) {
        printf("%s", oflag_str);
        oflag_str[0] = '\0';
        char * flags;
        if (c == 17)    flags = "--rdonly";
        else if(c == 18)flags = "--wronly";
        else            flags = "--rdwr";

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
        exit_status = MAX(exit_status,1);
        continue;
      }
      
      // save file descriptor to array
      if (fd_array_cur >= fd_array_size) {
        fd_array_size *= 2;
        //printf("fd_array reallocs %zu\n", fd_array_size);
        fd_array = (int*)realloc(fd_array, fd_array_size*sizeof(int)); 
        fd_isPipe = (int*)realloc(fd_isPipe, fd_array_size*sizeof(int));
      }
      fd_array[fd_array_cur] = rw_fd;
      fd_isPipe[fd_array_cur] = 0;
      fd_array_cur++;

      //clean oflag content.
      oflag = 0;
      if(profile){
        getrusage(RUSAGE_SELF, &usage_end);
        // long long end= (long long)usage.ru_utime.tv_sec*pow(10, 6) + (long long)usage.u_utime.tv_usec;
        long long endu = (long long ) (usage_end.ru_utime.tv_sec*pow(10, 6) +  usage_end.ru_utime.tv_usec);
        long long ends = (long long ) (usage_end.ru_stime.tv_sec*pow(10, 6) +  usage_end.ru_stime.tv_usec);
        // printf("endu = %lld\n", ends );
        printf("[profile] user CPU time => %lld us\t system CPU time => %lld us\n",
        endu - start_u, ends - start_s);
      }

      break;   
//pipe
    case 20:
      optind = findArgs(&args_array, args_array_size, optind, &args_array_cur,
                        argc, argv);

      if(verbose){
        printf("--pipe ");
        for(j = 0 ; j != args_array_cur; j++){
          printf("%s ", args_array[j]);
        }      
        printf("\n");
      }
      if(args_array_cur > 0 ){
        fprintf(stderr, "Error: --pipe does not take arguments.\n");
      }
      int pipefd[2]; //store indices of fd_array
      if(pipe(pipefd)== -1){
       fprintf(stderr, "Error: fail to create pipe. pipe() returns -1.\n");
      }
      // save file descriptor to array
      if (fd_array_cur  + 1 >= fd_array_size) {
        fd_array_size *= 2;
        fd_array = (int*)realloc(fd_array, fd_array_size*sizeof(int)); 
        fd_isPipe = (int*)realloc(fd_isPipe, fd_array_size*sizeof(int));
      }
      fd_isPipe[fd_array_cur] = 1;
      fd_array[fd_array_cur++] = pipefd[0];
      fd_isPipe[fd_array_cur] = 1;
      fd_array[fd_array_cur++] = pipefd[1];
      if(profile){
        getrusage(RUSAGE_SELF, &usage_end);
        // long long end= (long long)usage.ru_utime.tv_sec*pow(10, 6) + (long long)usage.u_utime.tv_usec;
        long long endu = (long long ) (usage_end.ru_utime.tv_sec*pow(10, 6) +  usage_end.ru_utime.tv_usec);
        long long ends = (long long ) (usage_end.ru_stime.tv_sec*pow(10, 6) +  usage_end.ru_stime.tv_usec);
        // printf("endu = %lld\n", ends );
        printf("[profile] user CPU time => %lld us\t system CPU time => %lld us\n",
        endu - start_u, ends - start_s);
      }
      break;

//close
    case 28:
      if(verbose){
        printf("--close %s\n", optarg);
      }
      if (!strIsNum(optarg)){
          fprintf(stderr, "Error: Incorrect usage of --close. Requires an integer argument.\n");
          exit_status = MAX(exit_status, 1);
          continue;
      }
      N = atoi(optarg);
      if(!validFd(N, fd_array_cur, &fd_array)){
        exit_status = MAX(exit_status, 1);
        continue;
      }
      close(fd_array[N]);
      fd_array[N] = -1;
      if(profile){
        getrusage(RUSAGE_SELF, &usage_end);
        // long long end= (long long)usage.ru_utime.tv_sec*pow(10, 6) + (long long)usage.u_utime.tv_usec;
        long long endu = (long long ) (usage_end.ru_utime.tv_sec*pow(10, 6) +  usage_end.ru_utime.tv_usec);
        long long ends = (long long ) (usage_end.ru_stime.tv_sec*pow(10, 6) +  usage_end.ru_stime.tv_usec);
        // printf("endu = %lld\n", ends );
        printf("[profile] user CPU time => %lld us\t system CPU time => %lld us\n",
        endu - start_u, ends - start_s);
      }
      break;
      
//verbose
    case 21: 
      verbose = 1;
      break;
//profile
    case 22:
      if(verbose){
        printf("--profile\n");
      }
      profile = 1;
      break;
//abort
    case 23:
      if(verbose) printf("--abort\n");
      // abort(); //no, because this is SIGABRT, not SIGSEGV
      //if --ignore 
      if(ignoreAbort) break;
      //this should cause sig_fault
      raise(11);
      // int *a = NULL;
      // int b = *a;
      break;
//catch
    case 24:
      if(verbose){
        printf("--catch %s\n", optarg);
      }
      if (!strIsNum(optarg)){
          fprintf(stderr, "Error: Incorrect usage of --catch. Requires an integer argument.\n");
          exit_status = MAX(exit_status, 1);
          continue;
      }
      N = atoi(optarg);
      sa.sa_sigaction = &catch_handler;
      sa.sa_flags = SA_SIGINFO;
      if (sigaction(N, &sa, NULL) < 0){
        /* Handle error */
          fprintf(stderr, "Error: fail to handle catch %d.\n",N);
          exit_status = MAX(exit_status, 1);
          continue;
      }
      
      // Test: --catch 11
      // //this should cause sig_fault
      // int *a = NULL;
      // int b = *a;
      if(profile){
        getrusage(RUSAGE_SELF, &usage_end);
        // long long end= (long long)usage.ru_utime.tv_sec*pow(10, 6) + (long long)usage.u_utime.tv_usec;
        long long endu = (long long ) (usage_end.ru_utime.tv_sec*pow(10, 6) +  usage_end.ru_utime.tv_usec);
        long long ends = (long long ) (usage_end.ru_stime.tv_sec*pow(10, 6) +  usage_end.ru_stime.tv_usec);
        // printf("endu = %lld\n", ends );
        printf("[profile] user CPU time => %lld us\t system CPU time => %lld us\n",
        endu - start_u, ends - start_s);
      }
      break;
//ignore
    case 25:
      if(verbose){
        printf("--ignore %s\n", optarg);
      }
      if (!strIsNum(optarg)){
          fprintf(stderr, "Error: Incorrect usage of --ignore. Requires an integer argument.\n");
          exit_status = MAX(exit_status, 1);
          continue;
      }
      N = atoi(optarg);
      if(N == 11)
        ignoreAbort = 1;
      sa.sa_handler = SIG_IGN;
      sa.sa_flags = 0;
      if (sigaction(N, &sa, NULL) < 0){
        /* Handle error */
          fprintf(stderr, "Error: fail to handle ignore %d.\n",N);
          exit_status = MAX(exit_status, 1);
          continue;
      }
      // //testing purpose
      // sleep(10);
      if(profile){
        getrusage(RUSAGE_SELF, &usage_end);
        // long long end= (long long)usage.ru_utime.tv_sec*pow(10, 6) + (long long)usage.u_utime.tv_usec;
        long long endu = (long long ) (usage_end.ru_utime.tv_sec*pow(10, 6) +  usage_end.ru_utime.tv_usec);
        long long ends = (long long ) (usage_end.ru_stime.tv_sec*pow(10, 6) +  usage_end.ru_stime.tv_usec);
        // printf("endu = %lld\n", ends );
        printf("[profile] user CPU time => %lld us\t system CPU time => %lld us\n",
        endu - start_u, ends - start_s);
      }
      break;
//default
    case 26:
      if(verbose){
        printf("--default %s\n", optarg);
      }
      if (!strIsNum(optarg)){
          fprintf(stderr, "Error: Incorrect usage of --default. Requires an integer argument.\n");
          exit_status = MAX(exit_status, 1);
          continue;
      }
      N = atoi(optarg);
      sa.sa_handler = SIG_DFL;
      sa.sa_flags = 0;
      if (sigaction(N, &sa, NULL) < 0){
        /* Handle error */
          fprintf(stderr, "Error: fail to handle default %d.\n",N);
          exit_status = MAX(exit_status, 1);
          continue;
      }
      if(profile){
        getrusage(RUSAGE_SELF, &usage_end);
        // long long end= (long long)usage.ru_utime.tv_sec*pow(10, 6) + (long long)usage.u_utime.tv_usec;
        long long endu = (long long ) (usage_end.ru_utime.tv_sec*pow(10, 6) +  usage_end.ru_utime.tv_usec);
        long long ends = (long long ) (usage_end.ru_stime.tv_sec*pow(10, 6) +  usage_end.ru_stime.tv_usec);
        // printf("endu = %lld\n", ends );
        printf("[profile] user CPU time => %lld us\t system CPU time => %lld us\n",
        endu - start_u, ends - start_s);
      }
      break;
//pause    
    case 27:
      if(verbose){
        printf("--pause\n");
      }

      pause();

      break;

    default:
        fprintf(stderr, "Error: ?? getopt returned character code 0%o ??\n", c);
    }
    free(args_array);
    // printf("done freeing\n");
  }

  // Close all used file descriptors
  fd_array_cur--;
  while (fd_array_cur >= 0) {
    if(fd_array[fd_array_cur] != -1)
    	close(fd_array[fd_array_cur]);
  	fd_array_cur--;
  }
  // Free dynamically allocated memory
  free(fd_array);
  free(fd_isPipe);
  free(wait_info);

  //[profile] 
  // get the ending usage of the whole program. Subtract this with the beginning usage data.
  if(profile){

    getrusage(RUSAGE_SELF, &usage_end);
    // long long end= (long long)usage.ru_utime.tv_sec*pow(10, 6) + (long long)usage.u_utime.tv_usec;
    long long endu = (long long )(usage_end.ru_utime.tv_sec*pow(10, 6) +  usage_end.ru_utime.tv_usec);
    long long ends = (long long )(usage_end.ru_stime.tv_sec*pow(10, 6) +  usage_end.ru_stime.tv_usec);
    printf("endu = %lld\n", ends );
    printf("[profile] Total...\n\t  user CPU time => %lld us\t system CPU time => %lld us\n",
      endu - begin_u + childrenTimeu, ends - begin_s + childrenTimes);
  }
  // printf("EXit with %d\n",exit_status );
  exit(exit_status);
}
