#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/select.h>

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
  int resultPipe[MAX_SLAVE_QTY][2];

  //Crear esclavos
  int slaveCount = 0;
  while (slaveCount < MAX_SLAVE_QTY && slaveCount < argc/MAX_INITIAL_FILES) {
    pipe(filePipe[slaveCount]);
    pipe(resultPipe[slaveCount]);

    pid = fork();

    if (pid == -1) {
      perror("fork");
      exit(-1);
    } 
    else if (pid == 0) {
      close(filePipe[slaveCount][1]);
      close(resultPipe[slaveCount][0]);
      dup2(filePipe[slaveCount][0], STDIN);
      dup2(resultPipe[slaveCount][1], STDOUT);
      char *argumentos[2] = {"/tmp", NULL};
      execvp("./slave.out", argumentos);
      perror("execvp");
      exit(-1);
    }
    else {
      close(filePipe[slaveCount][0]);
      close(resultPipe[slaveCount][1]);
      slaveCount++;
    }
  }
  
  fd_set fdSlaves;
  for(int nSlave = 0; nSlave < slaveCount; nSlave++)
    FD_SET(resultPipe[nSlave][0], &fdSlaves);

  int taskNum = 1;
  // Reparto inicial de tareas
  for (int i = 0; i < slaveCount; i++) {
    for (int j = 0; j < MAX_INITIAL_FILES && taskNum < argc; j++) {
      write(filePipe[i][1], argv[taskNum], strlen(argv[taskNum]));
      write(filePipe[i][1], "\n", 1);  
      taskNum++;
    }
  }

  int completedTasks = 0;

  while (completedTasks < argc-1) {
    
    // lees cuando alguno termina
    int slavesReady = select(resultPipe[slaveCount-1][1], &fdSlaves, NULL, NULL, NULL);
    if (slavesReady == -1)
      perror("select");
    else {

      for(int nSlave = 0; nSlave < slaveCount; nSlave++) {
        if(FD_ISSET(resultPipe[nSlave][0], &fdSlaves)) {
          completedTasks++;
          char buff[MAX_OUT_LEN];
          int readCount = read(resultPipe[nSlave][0], buff, MAX_OUT_LEN);
          buff[readCount] = 0;
          printf("%s\n", buff);

          if (taskNum < argc) {
            write(filePipe[nSlave][1], argv[taskNum], strlen(argv[taskNum]));
            write(filePipe[nSlave][1], "\n", 1);  
            taskNum++;
          }
        }
      }
    }
      
    FD_ZERO(&fdSlaves);
    for(int nSlave = 0; nSlave < slaveCount; nSlave++)
      FD_SET(resultPipe[nSlave][0], &fdSlaves);
  }

  for (int i = 0; i < slaveCount; i++) {
    close(resultPipe[i][0]);
    close(filePipe[i][1]);
  }
  
  waitpid(pid, NULL, 0);

}