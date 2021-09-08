#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/select.h>
#include "masterADT.h"

#define STDIN 0
#define STDOUT 1
#define MAX_SLAVE_QTY 5
#define MAX_INITIAL_FILES 1
#define MAX_OUT_LEN 512

typedef struct masterCDT {
  int filePipe[MAX_SLAVE_QTY][2];
  int resultPipe[MAX_SLAVE_QTY][2];

  int fileCount;
  const char **files;
  int slaveCount;
  int taskNum;
} masterCDT;

masterADT newMaster(const char *files[], int fileCount) {
  masterADT newMaster = calloc(1, sizeof(masterCDT));
  if(newMaster == NULL) {
    perror("malloc");
    exit(-1);
  }
  newMaster->fileCount = fileCount;
  newMaster->files = files;
  return newMaster;
}

void initializeSlaves(masterADT master) {
  pid_t pid;

  int slaveCount = 0;
  while (slaveCount < MAX_SLAVE_QTY && slaveCount < (master->fileCount+1)/MAX_INITIAL_FILES) {
    pipe(master->filePipe[slaveCount]);
    pipe(master->resultPipe[slaveCount]);

    pid = fork();

    if (pid == -1) {
      perror("fork");
      exit(-1);
    }
    else if (pid == 0) {
      close(master->filePipe[slaveCount][1]);
      close(master->resultPipe[slaveCount][0]);
      dup2(master->filePipe[slaveCount][0], STDIN);
      dup2(master->resultPipe[slaveCount][1], STDOUT);
      char *args[2] = {"/tmp", NULL};
      execvp("./slave.out", args);
      perror("execvp");
      exit(-1);
    }
    else {
      close(master->filePipe[slaveCount][0]);
      close(master->resultPipe[slaveCount][1]);
      slaveCount++;
    }
  }

  master->slaveCount = slaveCount;
}

void setInitialFiles(masterADT master) {
  int taskNum = 0;

  for (int i = 0; i < master->slaveCount; i++)
    for (int j = 0; j < MAX_INITIAL_FILES && taskNum < master->fileCount; j++) {
      write(master->filePipe[i][1], master->files[taskNum], strlen(master->files[taskNum]));
      write(master->filePipe[i][1], "\n", 1);  
      taskNum++;
    }

  master->taskNum = taskNum;
}

static void readResultPipe(int resultPipeEnd) {
  char buff[MAX_OUT_LEN];
  int readCount = read(resultPipeEnd, buff, MAX_OUT_LEN);
  buff[readCount] = 0;
  printf("%s\n", buff);
}

static void giveAnotherTask(int filePipeEnd, const char *file) {
  write(filePipeEnd, file, strlen(file));
  write(filePipeEnd, "\n", 1);
}

static void manageNewResults(masterADT master, int *completedTasks, fd_set fdSlaves) {
  for(int nSlave = 0; nSlave < master->slaveCount; nSlave++)
    if (FD_ISSET(master->resultPipe[nSlave][0], &fdSlaves)) {
      (*completedTasks)++;
      readResultPipe(master->resultPipe[nSlave][0]);

      if (master->taskNum < master->fileCount)
        giveAnotherTask(master->filePipe[nSlave][1], master->files[(master->taskNum)++]);
    }
}

void monitorSlaves(masterADT master) {
  fd_set fdSlaves;
  int completedTasks = 0;

  while (completedTasks < master->fileCount) {
    FD_ZERO(&fdSlaves);
    for(int nSlave = 0; nSlave < master->slaveCount; nSlave++)
      FD_SET(master->resultPipe[nSlave][0], &fdSlaves);
    
    if (select(master->resultPipe[master->slaveCount-1][1], &fdSlaves, NULL, NULL, NULL) == -1)
      perror("select");
    else
      manageNewResults(master, &completedTasks, fdSlaves);
  }
}

void closePipes(masterADT master){
  for (int i = 0; i < master->slaveCount; i++) {
    close(master->resultPipe[i][0]);
    close(master->filePipe[i][1]);
  }
}

void killMaster(masterADT master) {
  free(master);
}
