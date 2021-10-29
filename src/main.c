#include <stdio.h>

#include "cli.h"

int main(int argc, char **argv) {
  cli_t  cli = parse_cli_opts(argc, argv);

  fprintf(stdout, "watching dir: \"%s\"\n", cli.dir);
  fprintf(stdout, "port: \"%d\"\n", cli.port);

  return 0;
}

