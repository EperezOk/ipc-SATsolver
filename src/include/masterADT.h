#ifndef masterADT_h
#define masterADT_h

typedef struct masterCDT *masterADT;

masterADT newMaster(const char *files[], int fileCount, int shmKey);

void initializeSlaves(masterADT master);

void setInitialFiles(masterADT master);

void monitorSlaves(masterADT master);

void closeResources(masterADT master);

void freeMaster(masterADT master);

#endif