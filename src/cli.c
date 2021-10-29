#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include "cli.h"

char *version = "0.1.0";

char *short_options = "hqVd:p:";
struct option long_options[] = {
  { "help", no_argument, NULL, 'h' },
  { "quiet", no_argument, NULL, 'q' },
  { "version", no_argument, NULL, 'V' },
  { "dir", required_argument, NULL, 'd' },
  { "port", required_argument, NULL, 'p' },

  { 0, 0, 0, 0}
};

void print_help();
void print_version();

cli_t parse_cli_opts(int argc, char **argv) {
  int c;
  int option_index;

  cli_t cli = {
    .quiet = 0,
    .dir = ".",
    .port = 8080,
  };

  while (1) {
    c = getopt_long(argc, argv, short_options, long_options, &option_index);

    if (c == -1) break;

    switch (c) {
      case 'h':
        print_help();
        break;

      case 'q':
        cli.quiet = 1;
        break;

      case 'V':
        print_version();
        break;

      case 'd':
        cli.dir = optarg;
        break;

      case 'p':
        cli.port = atoi(optarg);
        break;

      case '?':
        break;

      default:
        fprintf(stderr, "invalid character code for options %o\n", c);
        exit(1);
    }
  }

  return cli;
}


void print_help() {
  fprintf(stdout, "web-watch [-d|--dir .] [-p|--port 8080] [-V|--version] [-h|--help] [-q|--quiet]\n");
  exit(0);
}

void print_version() {
  fprintf(stdout, "%s\n", version);
  exit(0);
}
