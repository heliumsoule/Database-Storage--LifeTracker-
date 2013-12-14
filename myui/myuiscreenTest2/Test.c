#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xterm_control.h"
#include "keyboard.h"
#include <unistd.h> 

char input[1000];

int ReadMystoreFromChild(char *argv1, char *argv2, char *argv3, char *argv4) {
  int pid, mypipe[2];
  char *newargv[6];
  char *errmsg;
  int n_input = 0;
  
  // turn off special keyboard handling
  getkey_terminate();
  
  // create the pipe
  if (pipe(mypipe) == -1) {
    strcpy(errmsg,"Problem in creating the pipe");
    return 0;
  }
  
  pid = fork();
  
  if (pid == -1) {
    strcpy(errmsg, "Error in forking");
    return 0;
  }
  if (pid == 0) {  // child
    close(mypipe[0]);  // Don't need to read from the pipe
    dup2(mypipe[1],STDOUT_FILENO);  // connect the "write-end" of the pipe to child's STDOUT
    
    newargv[0] = newargv[1] = "./mystore";
    newargv[2] = argv1;
    newargv[3] = argv2;
    newargv[4] = argv3;
    newargv[5] = argv4;
    execvp(newargv[0],newargv+1);
    exit(0);
  }
  else if (pid > 0) {
    char *s = input;
    int c;

    close(mypipe[1]);  // Don't need to write to the pipe
    
    // read the data into the input array from mypipe[0]
    FILE *fpin;
    if ((fpin = fdopen(mypipe[0],"r")) == NULL) {
      printf("ERROR: Cannot read from mypipe[0]\n");
      exit(1);
    }
    for (n_input = 0; n_input < sizeof(input)-1; ++n_input) {
      if ((c = getc(fpin)) == EOF) break;
      *s++ = c;
    }
    input[n_input] = '\0';
    fclose(fpin);
    
    wait(NULL);  // wait for child to finish
    close(mypipe[0]);
  }
  
  return n_input;
}

int main() {
    int Count = ReadMyStoreFromChild("stat", NULL, NULL, NULL);
    }
