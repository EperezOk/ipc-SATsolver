#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define STDIN 0
#define MAX_LEN 15

typedef struct shmseg {
  int count;
  char buff[MAX_LEN];
} shmseg;


// DEBE recibir por entrada estándar y como parámetro la información necesaria para
// conectarse al buffer compartido. 
// Esto DEBERÁ permitir utilizar un pipe para iniciar el proceso aplicación y el 
// vista: ./solve files/* | ./vista, y también iniciar la aplicación y
// más tarde la vista: ./solve files/* en una terminal o en background y ./vista <info> en
// otra terminal o en foreground.


int main(int argc, const char *argv[]) {
    struct shmseg *shmp;
    sleep(1);

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

    printf("%s\n", shmp->buff);

    if (shmdt(shmp) == -1)
      perror("shmdt");

    if (shmctl(shmid, IPC_RMID, 0) == -1)
      perror("shmctl");
  
}
