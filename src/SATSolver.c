#include "masterADT.h"
#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <unistd.h>

// SHM Include
#include <sys/ipc.h>
#include <sys/stat.h>
#include <sys/shm.h>
// SEM include
#include <semaphore.h>
#include <fcntl.h>

#define SEM_NAME "/semAppView"
#define OBJ_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)
#define BUF_SIZE 1024

#define handle_error(msg) \
           do { perror(msg); exit(EXIT_FAILURE); } while (0)

typedef struct SharedMem {
  char buff[BUF_SIZE];
} SharedMem;

int main(int argc, const char *argv[]) {
  
  if (argc < 2) {
    printf("Please provide the files to process.\n");
    exit(-1);
  }

  int sharedMemKey = getpid(), sharedMemID;
  struct SharedMem *sharedMem;

  /* Create shared memory; attach at address chosen by system */

  sharedMemID = shmget(sharedMemKey, sizeof(struct SharedMem), IPC_CREAT | OBJ_PERMS);
  printf("%d", sharedMemID);
  if (sharedMemID == -1)
    handle_error("shmget");

  sharedMem = shmat(sharedMemID, NULL, 0);
  if (sharedMem == (void *) -1)
    handle_error("shmat");

  // SEMAPHORE - CHECKEAR ERRORES

  sem_t *semaph = sem_open(SEM_NAME, O_CREAT, 0660, 1); // initialValue = 1
  if (semaph == SEM_FAILED)
    handle_error("sem_open");
  
  if (sem_wait(semaph) == -1)
    handle_error("sem_wait");

  sprintf(sharedMem->buff, "TESTING\n");

  if (sem_post(semaph) == -1)
    handle_error("sem_post");
  
  if (sem_close(semaph) == -1)
    handle_error("sem_close");

  if (shmdt(sharedMem) == -1)
    handle_error("shmdt");
  
  // masterADT master = nuevoMaster(argv+1, argc-1);

  // initializeSlaves(master);
  // setInitialFiles(master);
  // monitorSlaves(master);
  // closePipes(master);
  // killMaster(master);
}