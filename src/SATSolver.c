// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include "./include/masterADT.h"
#include "./include/shMemHandlerADT.h"

int main(int argc, const char *argv[]) {
  
  if (argc < 2) {
    printf("Please provide the files to process.\n");
    exit(EXIT_FAILURE);
  }
  
  masterADT master = newMaster(argv+1, argc-1, getpid());

  initializeSlaves(master);
  setInitialFiles(master);
  monitorSlaves(master);
  closeResources(master);
  freeMaster(master);

  return 0;
}