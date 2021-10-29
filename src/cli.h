#ifndef cli_h
#define cli_h

typedef struct {
  int quiet;
  char *dir;
  int port;
} cli_t;

cli_t parse_cli_opts(int, char**);

#endif
