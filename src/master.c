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

int initializeSlaves(int filePipe[MAX_SLAVE_QTY][2], int resultPipe[MAX_SLAVE_QTY][2], int fileCount);
int setInitialFiles(int slaveCount, int filePipe[MAX_SLAVE_QTY][2], char const* files[], int fileCount);
void monitorSlaves(char const* files[], int fileCount, int slaveCount, int taskNum, int filePipe[MAX_SLAVE_QTY][2], int resultPipe[MAX_SLAVE_QTY][2]);
void readResultPipe(int resultPipeEnd);
void giveAnotherTask(int filePipeEnd, const char *file);
void closePipes(int slaveCount, int filePipe[MAX_SLAVE_QTY][2], int resultPipe[MAX_SLAVE_QTY][2]);
void manageNewResults(int *completedTasks, int slaveCount, int *taskNum, const char *files[], int fileCount, int filePipe[MAX_SLAVE_QTY][2], int resultPipe[MAX_SLAVE_QTY][2], fd_set fdSlaves);

int main (int argc, char const *argv[]) {

  if (argc < 2) {
    printf("Please provide the files to process.\n");
    exit(-1);
  }

  int filePipe[MAX_SLAVE_QTY][2];
  int resultPipe[MAX_SLAVE_QTY][2];

  int slaveCount = initializeSlaves(filePipe, resultPipe, argc-1);
  int taskNum = setInitialFiles(slaveCount, filePipe, argv, argc);

  monitorSlaves(argv, argc, slaveCount, taskNum, filePipe, resultPipe);
  closePipes(slaveCount, filePipe, resultPipe);

}

int initializeSlaves(int filePipe[MAX_SLAVE_QTY][2], int resultPipe[MAX_SLAVE_QTY][2], int fileCount) {
  pid_t pid;

  int slaveCount = 0;
  while (slaveCount < MAX_SLAVE_QTY && slaveCount < (fileCount+1)/MAX_INITIAL_FILES) {
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
      char *args[2] = {"/tmp", NULL};
      execvp("./slave.out", args);
      perror("execvp");
      exit(-1);
    }
    else {
      close(filePipe[slaveCount][0]);
      close(resultPipe[slaveCount][1]);
      slaveCount++;
    }
  }
  
  return slaveCount;
}

int setInitialFiles(int slaveCount, int filePipe[MAX_SLAVE_QTY][2], char const* files[], int fileCount){
  int taskNum = 1;

  for (int i = 0; i < slaveCount; i++)
    for (int j = 0; j < MAX_INITIAL_FILES && taskNum < fileCount; j++) {
      write(filePipe[i][1], files[taskNum], strlen(files[taskNum]));
      write(filePipe[i][1], "\n", 1);  
      taskNum++;
    }

  return taskNum;
}

void monitorSlaves(char const* files[], int fileCount, int slaveCount, int taskNum, int filePipe[MAX_SLAVE_QTY][2], int resultPipe[MAX_SLAVE_QTY][2]) {
  fd_set fdSlaves;
  int completedTasks = 0;

  while (completedTasks < fileCount-1) {
    FD_ZERO(&fdSlaves);
    for(int nSlave = 0; nSlave < slaveCount; nSlave++)
      FD_SET(resultPipe[nSlave][0], &fdSlaves);
    
    if (select(resultPipe[slaveCount-1][1], &fdSlaves, NULL, NULL, NULL) == -1)
      perror("select");
    else
      manageNewResults(&completedTasks, slaveCount, &taskNum, files, fileCount, filePipe, resultPipe, fdSlaves);
  }
}

void manageNewResults(int *completedTasks, int slaveCount, int *taskNum, const char *files[], int fileCount, int filePipe[MAX_SLAVE_QTY][2], int resultPipe[MAX_SLAVE_QTY][2], fd_set fdSlaves) {
  for(int nSlave = 0; nSlave < slaveCount; nSlave++)
    if (FD_ISSET(resultPipe[nSlave][0], &fdSlaves)) {
      (*completedTasks)++;
      readResultPipe(resultPipe[nSlave][0]);

      if (*taskNum < fileCount)
        giveAnotherTask(filePipe[nSlave][1], files[(*taskNum)++]);
    }
}

void readResultPipe(int resultPipeEnd) {
  char buff[MAX_OUT_LEN];
  int readCount = read(resultPipeEnd, buff, MAX_OUT_LEN);
  buff[readCount] = 0;
  printf("%s\n", buff);
}

void giveAnotherTask(int filePipeEnd, const char *file) {
  write(filePipeEnd, file, strlen(file));
  write(filePipeEnd, "\n", 1);
}

void closePipes(int slaveCount, int filePipe[MAX_SLAVE_QTY][2], int resultPipe[MAX_SLAVE_QTY][2]){
  for (int i = 0; i < slaveCount; i++) {
    close(resultPipe[i][0]);
    close(filePipe[i][1]);
  }
}