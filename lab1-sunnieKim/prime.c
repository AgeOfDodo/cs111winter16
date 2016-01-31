//big computation program for benchmarks

#include <stdio.h>
#include <stdlib.h>

int isPrimeNumber(unsigned long int n){
  int i;
  for(i=2; i<=(n>>1); i++)
    if(n%i==0) return 0;
  return 1;
}

int main(int argc, char *argv[]){
  unsigned long int argument = strtoul(argv[1], NULL, 10), n = argument;
  while(!isPrimeNumber(++n));

  printf("Prime number greater than %ld is %ld\n", argument, n);
  return 0;
}