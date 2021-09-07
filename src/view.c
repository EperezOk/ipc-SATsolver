#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include <semaphore.h>
#include <fcntl.h>

#include <sys/stat.h>
#include <sys/types.h>

#define STDIN 0
#define MAX_LEN 15

typedef struct shmseg {
  char buff[MAX_LEN];
} shmseg;

int main(int argc, const char *argv[]) {
  struct shmseg *shmp;

  int shmid;

  if (argc == 2)
    shmid = atoi(argv[1]);
  else {
    char buff[MAX_LEN + 1];
    int count = read(STDIN, buff, MAX_LEN);
    buff[count] = 0;
    shmid = atoi(buff);
  }

  shmp = shmat(shmid, NULL, 0);
  if (shmp == (void *) -1)
    perror("shmat");


  // SEMAPHORE - CHECKEAR ERRORES
  sem_t *semaph = sem_open("/semAppView", O_CREAT);
  if (semaph == SEM_FAILED) {
    perror("sem_open");
    exit(-1);
  }

  sem_wait(semaph);
  printf("%s\n", shmp->buff);
  sem_post(semaph);

  sem_close(semaph);
  sem_unlink("/semAppView");

  if (shmdt(shmp) == -1)
    perror("shmdt");

  if (shmctl(shmid, IPC_RMID, 0) == -1)
    perror("shmctl");


  
}
