// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/select.h>
#include "./include/masterADT.h"
#include "./include/shMemHandlerADT.h"

#define STDIN 0
#define STDOUT 1
#define MAX_SLAVE_QTY 5
#define MAX_INITIAL_FILES 1
#define MAX_OUT_LEN 256
#define RESULT_PATH "./results.txt"
#define handle_error(msg) \
           do { perror(msg); exit(EXIT_FAILURE); } while (0)

typedef struct masterCDT {
  int filePipe[MAX_SLAVE_QTY][2];
  int resultPipe[MAX_SLAVE_QTY][2];
  int filesRead[MAX_SLAVE_QTY];

  int fileCount;
  const char **files;
  int slaveCount;
  int taskNum;

  shMemHandlerADT shMemHandler;
  int shMemID;
} masterCDT;

masterADT newMaster(const char *files[], int fileCount, int shmKey) {
  masterADT newMaster = calloc(1, sizeof(masterCDT));
  if(newMaster == NULL)
    handle_error("malloc");
  newMaster->fileCount = fileCount;
  newMaster->files = files;

  newMaster->shMemHandler = newShMemHandler();
  newMaster->shMemID = initShMem(getpid());
  attachTo(newMaster->shMemHandler, newMaster->shMemID, 0);

  setvbuf(stdout, NULL, _IONBF, 0);
  printf("%d", newMaster->shMemID);

  return newMaster;
}

void initializeSlaves(masterADT master) {
  pid_t pid;

  int slaveCount = 0, i;
  while (slaveCount < MAX_SLAVE_QTY && slaveCount < (master->fileCount+1)/MAX_INITIAL_FILES) {
    pipe(master->filePipe[slaveCount]);
    pipe(master->resultPipe[slaveCount]);

    pid = fork();

    if (pid == -1)
      handle_error("fork");
    else if (pid == 0) {
      for (i = 0; i < slaveCount; i++) {
        close(master->filePipe[i][1]);
        close(master->resultPipe[i][0]);
      }

      close(master->filePipe[slaveCount][1]);
      close(master->resultPipe[slaveCount][0]);

      dup2(master->filePipe[slaveCount][0], STDIN);
      dup2(master->resultPipe[slaveCount][1], STDOUT);

      close(master->filePipe[slaveCount][0]);
      close(master->resultPipe[slaveCount][1]);

      char *args[2] = {"/tmp", NULL};
      execvp("./slave.out", args);
      handle_error("execvp");
    }
    else {
      master->filesRead[slaveCount] = 0;
      close(master->filePipe[slaveCount][0]);
      close(master->resultPipe[slaveCount][1]);
      slaveCount++;
    }
  }

  master->slaveCount = slaveCount;
}

static void giveAnotherTask(int filePipeEnd, const char *file) {
  write(filePipeEnd, file, strlen(file));
  write(filePipeEnd, "\n", 1);
}

void setInitialFiles(masterADT master) {
  int taskNum = 0, i, j;
  for (i = 0; i < master->slaveCount; i++)
    for (j = 0; j < MAX_INITIAL_FILES && taskNum < master->fileCount; j++) {
      giveAnotherTask(master->filePipe[i][1], master->files[taskNum]);
      taskNum++;
    }
  master->taskNum = taskNum;
}

static void readResultPipe(masterADT master, int resultPipeEnd, FILE *resultFile, int completedTasks) {
  char buff[MAX_OUT_LEN + 1];
  int readCount = read(resultPipeEnd, buff, MAX_OUT_LEN);
  if (readCount == -1)
    handle_error("read");
  buff[readCount] = 0;
  
  if (completedTasks == master->fileCount)
    finishWriting(master->shMemHandler);
  writeShMem(master->shMemHandler, buff);

  buff[readCount] = '\n';
  if (fwrite(buff, sizeof(char), readCount+1, resultFile) == 0)
    handle_error("fwrite");
}

static void manageNewResults(masterADT master, int *completedTasks, fd_set fdSlaves, FILE *resultFile) {
  int nSlave;
  for(nSlave = 0; nSlave < master->slaveCount; nSlave++)
    if (FD_ISSET(master->resultPipe[nSlave][0], &fdSlaves)) {
      (*completedTasks)++;
      readResultPipe(master, master->resultPipe[nSlave][0], resultFile, *completedTasks);
      master->filesRead[nSlave]++;

      if (master->taskNum < master->fileCount && master->filesRead[nSlave] >= MAX_INITIAL_FILES)
        giveAnotherTask(master->filePipe[nSlave][1], master->files[(master->taskNum)++]);
    }
}

void monitorSlaves(masterADT master) {
  fd_set fdSlaves;
  int completedTasks = 0, nSlave;

  FILE *resultFile = fopen(RESULT_PATH,"w");
  if(resultFile == NULL)
    handle_error("fopen");

  while (completedTasks < master->fileCount) {
    FD_ZERO(&fdSlaves);
    for(nSlave = 0; nSlave < master->slaveCount; nSlave++)
      FD_SET(master->resultPipe[nSlave][0], &fdSlaves);

    if (select(master->resultPipe[master->slaveCount-1][1], &fdSlaves, NULL, NULL, NULL) == -1)
      handle_error("select");
    else
      manageNewResults(master, &completedTasks, fdSlaves, resultFile);
  }

  if (fclose(resultFile) == -1)
    handle_error("fclose");
}

static void closePipes(masterADT master){
  int i;
  for (i = 0; i < master->slaveCount; i++) {
    close(master->resultPipe[i][0]);
    close(master->filePipe[i][1]);
  }
}

void closeResources(masterADT master){
  closeShMem(master->shMemHandler);
  closePipes(master);
}


void freeMaster(masterADT master) {
  destroyShMem(master->shMemHandler, master->shMemID);
  freeHandler(master->shMemHandler);
  free(master);
}
