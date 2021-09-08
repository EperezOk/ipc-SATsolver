#ifndef masterADT_h
#define masterADT_h

typedef struct masterCDT *masterADT;

masterADT newMaster(const char *files[], int fileCount);

void initializeSlaves(masterADT master);

void setInitialFiles(masterADT master);

void monitorSlaves(masterADT master);

void closePipes(masterADT master);

void killMaster(masterADT master);

#endif