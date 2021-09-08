#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "shMemHandlerADT.h"

#define STDIN 0
#define MAX_ID_LEN 15
#define MAX_OUTPUT_LEN 1024

int getIdFromStdin();

int main(int argc, const char *argv[]) {

  int shMemID;

  if (argc == 2)
    shMemID = atoi(argv[1]);
  else 
    shMemID = getIdFromStdin();

  shMemHandlerADT shMemHandler = newShMemHandler();

  attachTo(shMemHandler, shMemID);
  char output[MAX_OUTPUT_LEN+1];
  readShMem(shMemHandler, output);
  printf("%s\n", output);
  closeShMem(shMemHandler);
  freeHandler(shMemHandler);

  return 0;
}

int getIdFromStdin() {
  char buff[MAX_ID_LEN + 1];
  int count = read(STDIN, buff, MAX_ID_LEN);
  buff[count] = 0;
  return atoi(buff);
}
