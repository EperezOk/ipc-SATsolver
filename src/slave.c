#include <stdio.h>
#include <stdlib.h>
#define STDIN 0
#define STDOUT 1
#define MAX_LEN 8192
#define SAT_SOLVER "minisat"

#include <unistd.h>

// Recibe los paths de el/los archivos a procesar. Tiene que recibir los paths por stdin e imprimir por stdout, despues se cambian los fd a los pipes correspondientes en el master.

int main(int argc, char const *argv[]) {
  /*
  Para pasar muchos paths de una podemos hacer "path1\npath2\n\path3" y luego ir recopilando los distintos paths con getline(), checkeando errno para ver si llegamos al final del archivo (EOF).
  */

  char *line = NULL;
  size_t len = 0;
  ssize_t nread;

  while ((nread = getline(&line, &len, stdin)) > 0) {
    line[nread-1] = 0;
    
    char command[MAX_LEN + 1];
    char output[MAX_LEN + 1];
    sprintf(command, "%s %s | grep -o -e \"Number of.*[0-9]\\+\" -e \"CPU time.*\" -e \".*SATISFIABLE\"", SAT_SOLVER, line);
    
    FILE *outputStream;
    if ((outputStream = popen(command, "r")) == NULL)
      perror("popen");

    int count = fread(output, sizeof(char), MAX_LEN, outputStream);
    output[count] = 0;

    write(STDOUT, output, count);

    if (pclose(outputStream) == -1)
      perror("pclose");
  }

  free(line);
}