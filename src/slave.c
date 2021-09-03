#include <stdio.h>
#include <stdlib.h>

// Recibe los paths de el/los archivos a procesar. Tiene que recibir los paths por stdin e imprimir por stdout, despues se cambian los fd a los pipes correspondientes en el master.

int main(int argc, char const *argv[]) {
  /*
  Para pasar muchos paths de una podemos hacer "path1\npath2\n\path3" y luego ir recopilando los distintos paths con getline(), checkeando errno para ver si llegamos al final del archivo (EOF).
  */

  char *line = NULL;
  size_t len = 0;
  ssize_t nread;

  while ((nread = getline(&line, &len, stdin)) > 0) {
    // FILE *cnfFile = fopen(line, "r");
    // if (cnfFile == NULL) {
    //   perror("fopen");
    // }

    printf("%s\n", line);
  }

  free(line);
}