#include "masterADT.h"
#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <unistd.h>

// SHM Include
#include <sys/ipc.h>
#include <sys/stat.h>
#include <sys/shm.h>

#define OBJ_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)
#define BUF_SIZE 1024
#define SEM_KEY 0x5678

typedef struct shmseg {
  int count;
  char buff[BUF_SIZE];
} shmseg;

union semun {
  int              val;    /* Value for SETVAL */
  struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
  unsigned short  *array;  /* Array for GETALL, SETALL */
  struct seminfo  *__buf;  /* Buffer for IPC_INFO (Linux-specific) */
};

int main(int argc, const char *argv[]) {
  
  if (argc < 2) {
    printf("Please provide the files to process.\n");
    exit(-1);
  }

  union semun dummy;

  int shmkey = getpid(), shmid, semid;
  struct shmseg *shmp;

  /* Create set containing two semaphores; initialize so that writer has first access to shared memory. */

  semid = semget(SEM_KEY, 2, IPC_CREAT | OBJ_PERMS);
  if (semid == -1)
    perror("semget");

  /*
  FALTA INICIALIZAR EL SEMAFORO, SEMGET O CAPAZ SEM_OPEN

  if (initSemAvailable(semid, WRITE_SEM) == -1)
    perror("initSemAvailable");
  if (initSemInUse(semid, READ_SEM) == -1)
    perror("initSemInUse");

  semctl(semid, 0, IPC_SET | ) // VER SETVAL EN VEZ DE IPC_SET
  */

  /* Create shared memory; attach at address chosen by system */

  shmid = shmget(shmkey, sizeof(struct shmseg), IPC_CREAT | OBJ_PERMS);
  printf("SHMID: %d\n", shmid);
  if (shmid == -1)
    perror("shmget");

  shmp = shmat(shmid, NULL, 0);
  if (shmp == (void *) -1)
    perror("shmat");

  shmp->count = sprintf(shmp->buff, "TESTING\n");

  if (shmdt(shmp) == -1)
    perror("shmdt");
  
  masterADT master = nuevoMaster(argv+1, argc-1);

  initializeSlaves(master);
  setInitialFiles(master);
  monitorSlaves(master);
  closePipes(master);
  killMaster(master);
}