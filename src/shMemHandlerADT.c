#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <fcntl.h>
#include "shMemHandlerADT.h"

#include <unistd.h>

#define BUF_SIZE 1024
#define SEM_NAME "/semAppView"
#define OBJ_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)
#define handle_error(msg) \
           do { perror(msg); exit(EXIT_FAILURE); } while (0)

typedef struct SharedMem {
  char buff[BUF_SIZE];
} SharedMem;

typedef struct shMemHandlerCDT {
  SharedMem *shMem;
  sem_t *semaph;
} shMemHandlerCDT;

shMemHandlerADT newShMemHandler() {
  shMemHandlerADT shMemHandler = calloc(1, sizeof(shMemHandlerCDT));
  if(shMemHandler == NULL)
    handle_error("calloc");

  shMemHandler->semaph = sem_open(SEM_NAME, O_CREAT, 0660, 1);
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

void attachTo(shMemHandlerADT shMemHandler, int shMemID) {
  shMemHandler->shMem = (SharedMem *) shmat(shMemID, NULL, 0);
  if (shMemHandler->shMem == (void *) -1)
    handle_error("shmat");
}

void writeShMem(shMemHandlerADT shMemHandler, const char *msg) {
  if (sem_wait(shMemHandler->semaph) == -1)
    handle_error("sem_wait");

  snprintf(shMemHandler->shMem->buff, BUF_SIZE, "%s", msg);
  sleep(10);

  if (sem_post(shMemHandler->semaph) == -1)
    handle_error("sem_post");
}

void readShMem(shMemHandlerADT shMemHandler, char *buff) {
  sem_wait(shMemHandler->semaph);
  snprintf(buff, BUF_SIZE, "%s", shMemHandler->shMem->buff);
  sem_post(shMemHandler->semaph);
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
