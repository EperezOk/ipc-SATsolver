#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define STDIN 0
#define STDOUT 1

#define MAX_OUT_LEN 512

int main (int argc, char const *argv[]) {
  if (argc < 2) {
    printf("Please provide the files to process.\n");
    exit(-1);
  }

  pid_t pid;
  int filePipe[2];
  int resultPipe[2];

  pipe(filePipe);
  pipe(resultPipe);
  pid = fork();

  if (pid == -1) {
    perror("fork");
    exit(-1);
  } 
  else if (pid == 0) {  // Hijo
    close(filePipe[1]);
    close(resultPipe[0]);
    dup2(filePipe[0], STDIN);
    dup2(resultPipe[1], STDOUT);
    char *argumentos[2] = {"/tmp", NULL};
    execvp("./slave.out", argumentos);
    perror("execvp");
    exit(-1);
  }
  else { // Padre 
    close(filePipe[0]);
    close(resultPipe[1]);
    for (int i = 1; i < argc; i++) {
      write(filePipe[1], argv[i], strlen(argv[i]));
      write(filePipe[1], "\n", 1);  
    }
    close(filePipe[1]);
    char buff[MAX_OUT_LEN];
    while (read(resultPipe[0], buff, MAX_OUT_LEN) > 0) {
      printf("%s\n", buff);
    }
  }
  
  close(resultPipe[0]);
  
  waitpid(pid, NULL, 0);

}