#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>

#define STDIN 0
#define STDOUT 1
#define MAX_LEN 1024
#define SAT_SOLVER "minisat"

FILE *executeSolver(char *fileName);
void printResult(char *fileName, FILE *outputStream);

int main(int argc, char const *argv[]) {

  char *line = NULL;
  size_t len = 0;
  ssize_t nread;

  while ((nread = getline(&line, &len, stdin)) > 0) {
    line[nread-1] = 0;
    
    FILE *outputStream = executeSolver(line);
    printResult(line, outputStream);

    if (pclose(outputStream) == -1)
      perror("pclose");
  }

  free(line);
  return 1;
}

FILE *executeSolver(char *fileName) {
  char command[MAX_LEN + 1];
  sprintf(command, "%s %s | grep -o -e \"Number of.*[0-9]\\+\" -e \"CPU time.*\" -e \".*SATISFIABLE\"", SAT_SOLVER, fileName);
  
  FILE *outputStream;
  if ((outputStream = popen(command, "r")) == NULL)
    perror("popen");

  return outputStream;
}

void printResult(char *fileName, FILE *outputStream) {
  char output[MAX_LEN + 1];

  int count = fread(output, sizeof(char), MAX_LEN, outputStream);
  output[count] = 0;
  int fileNameLength = strlen(fileName);
  strcat(output, "File: ");
  strcat(output, fileName);
  strcat(output, "\n");

  char slaveIDString[20];

  pid_t slaveID = getpid();
  count += snprintf(slaveIDString, 20, "Slave ID: %d\n", slaveID) + fileNameLength + 7;
  strcat(output, slaveIDString);
  output[count] = 0;

  write(STDOUT, output, count);
}