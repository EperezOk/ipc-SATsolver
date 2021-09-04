#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define STDIN 0
#define STDOUT 1
#define MAX_SLAVE_QTY 5
#define MAX_INITIAL_FILES 1
#define MAX_OUT_LEN 512

int main (int argc, char const *argv[]) {
  if (argc < 2) {
    printf("Please provide the files to process.\n");
    exit(-1);
  }

  pid_t pid;
  int filePipe[MAX_SLAVE_QTY][2];
  int resultPipe[2];

  pipe(resultPipe);
  close(resultPipe[1]);

  int slaveCount = 0;
  while (slaveCount < MAX_SLAVE_QTY && slaveCount < argc/MAX_INITIAL_FILES) {
    pipe(filePipe[slaveCount]);

    pid = fork();

    if (pid == -1) {
      perror("fork");
      exit(-1);
    } 
    else if (pid == 0) {
      close(filePipe[slaveCount][1]);
      dup2(filePipe[slaveCount][0], STDIN);
      dup2(resultPipe[1], STDOUT);
      char *argumentos[2] = {"/tmp", NULL};
      execvp("./slave.out", argumentos);
      perror("execvp");
      exit(-1);
    }
    else {
      close(filePipe[0]);
      slaveCount++;
    }
  }
  
  int taskNum = 1;
  // Reparto inicial de tareas
  for (int i = 0; i < slaveCount; i++) {
    for (int j = 0; j < MAX_INITIAL_FILES && taskNum < argc; j++) {
      write(filePipe[i][1], argv[taskNum], strlen(argv[taskNum]));
      write(filePipe[i][1], "\n", 1);  
      taskNum++;
    }
  }

  close(filePipe[1]);
  char buff[MAX_OUT_LEN];
  while (read(resultPipe[0], buff, MAX_OUT_LEN) > 0) {
    printf("%s\n", buff);
  }
  
  close(resultPipe[0]);

  waitpid(pid, NULL, 0);

}