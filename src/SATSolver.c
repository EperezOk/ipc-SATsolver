#include "masterADT.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, const char *argv[]) {
  
  if (argc < 2) {
    printf("Please provide the files to process.\n");
    exit(-1);
  }
  
  masterADT master = nuevoMaster(argv+1, argc-1);

  initializeSlaves(master);
  setInitialFiles(master);
  monitorSlaves(master);
  closePipes(master);
  killMaster(master);
}