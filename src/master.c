#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define STDIN 0

int main (int argc, char const *argv[]) {
  if (argc < 2) {
    printf("Please provide the files to process.\n");
    exit(-1);
  }

  pid_t pid;
  int pipeEnds[2];

  pipe(pipeEnds);
  pid = fork();

  if (pid == -1) {
    perror("fork");
    exit(-1);
  }
  else if (pid == 0) {
    close(pipeEnds[1]);
    dup2(pipeEnds[0], STDIN);
    char *argumentos[2] = {"/tmp", NULL};
    execvp("./slave", argumentos);
    perror("execvp");
    exit(-1);
  }
  else {
    close(pipeEnds[0]);
    for (int i = 1; i < argc; i++) {
      write(pipeEnds[1], argv[i], strlen(argv[i]));
      write(pipeEnds[1], "\n", 1);
      sleep(2);
    } 
    close(pipeEnds[1]);
  }

  waitpid(pid, NULL, 0);
 }