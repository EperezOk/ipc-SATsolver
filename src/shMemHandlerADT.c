// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <fcntl.h>
#include "./include/shMemHandlerADT.h"

#define BUF_SIZE 256
#define MAX_SLOTS 25
#define SEM_NAME "/semAppView"
#define OBJ_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)
#define handle_error(msg) \
           do { perror(msg); exit(EXIT_FAILURE); } while (0)

typedef struct SharedMem {
  int currentWriteSlot;
  int currentReadSlot;
  int finishedWriting;
  char buff[MAX_SLOTS][BUF_SIZE];
} SharedMem;

typedef struct shMemHandlerCDT {
  SharedMem *shMem;
  sem_t *semaph;
} shMemHandlerCDT;

shMemHandlerADT newShMemHandler() {
  shMemHandlerADT shMemHandler = calloc(1, sizeof(shMemHandlerCDT));
  if(shMemHandler == NULL)
    handle_error("calloc");

  shMemHandler->semaph = sem_open(SEM_NAME, O_CREAT, 0660, 0);
  if (shMemHandler->semaph == SEM_FAILED)
    handle_error("sem_open");

  return shMemHandler;
}

int initShMem(int key) {
  int shMemID = shmget(key, sizeof(struct SharedMem), IPC_CREAT | OBJ_PERMS);
  if (shMemID == -1)
    handle_error("shmget");
  return shMemID;
}

void attachTo(shMemHandlerADT shMemHandler, int shMemID, int reader) {
  shMemHandler->shMem = (SharedMem *) shmat(shMemID, NULL, 0);
  if (shMemHandler->shMem == (void *) -1)
    handle_error("shmat");
  if (reader)
    shMemHandler->shMem->currentReadSlot = 0;
  else {
    shMemHandler->shMem->currentWriteSlot = 0;
    shMemHandler->shMem->finishedWriting = 0;
  }
}

void writeShMem(shMemHandlerADT shMemHandler, const char *msg) {
  if (shMemHandler->shMem->currentWriteSlot == MAX_SLOTS) {
    printf("Out of memory for results\n");
    exit(EXIT_FAILURE);
  }
  snprintf(shMemHandler->shMem->buff[shMemHandler->shMem->currentWriteSlot++], BUF_SIZE, "%s", msg);
  
  if (sem_post(shMemHandler->semaph) == -1)
    handle_error("sem_post");
}

void readShMem(shMemHandlerADT shMemHandler, char *buff) {
  if (sem_wait(shMemHandler->semaph) == -1)
    handle_error("sem_post");
  int n;
  sem_getvalue(shMemHandler->semaph, &n);
  printf("%d\n",n);
  snprintf(buff, BUF_SIZE, "%s", shMemHandler->shMem->buff[shMemHandler->shMem->currentReadSlot++]);
}

void finishWriting(shMemHandlerADT shMemHandler) {
  shMemHandler->shMem->finishedWriting = 1;
}

int canRead(shMemHandlerADT shMemHandler) {
  return !shMemHandler->shMem->finishedWriting;
}

void closeShMem(shMemHandlerADT shMemHandler) {
  if (sem_close(shMemHandler->semaph) == -1)
    handle_error("sem_close");
  if (shmdt(shMemHandler->shMem) == -1)
    handle_error("shmdt");
}

void destroyShMem(shMemHandlerADT shMemHandler, int shMemID) {
  if (shmctl(shMemID, IPC_RMID, 0) == -1)
    handle_error("shmctl");
  sem_unlink(SEM_NAME);
}

void freeHandler(shMemHandlerADT shMemHandler) {
  free(shMemHandler);
}
