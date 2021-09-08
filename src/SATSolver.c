#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include "masterADT.h"
#include "shMemHandlerADT.h"

int main(int argc, const char *argv[]) {
  
  if (argc < 2) {
    printf("Please provide the files to process.\n");
    exit(-1);
  }

  shMemHandlerADT shMemHandler = newShMemHandler();

  int shMemID = initShMem(getpid());

  setvbuf(stdout, NULL, _IONBF, 0);
  printf("%d", shMemID);

  attachTo(shMemHandler, shMemID);
  writeShMem(shMemHandler, "TESTING");
  closeShMem(shMemHandler);

  destroyShMem(shMemHandler, shMemID);
  freeHandler(shMemHandler);
  
  // masterADT master = newMaster(argv+1, argc-1);

  // initializeSlaves(master);
  // setInitialFiles(master);
  // monitorSlaves(master);
  // closePipes(master);
  // killMaster(master);

  return 0;
}