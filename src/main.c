#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <dbg.h>
#include <sys/select.h>

#include "cli.h"
#include "watch.h"

void sig_cleanup(int);
int watch_fd = -1;

int main(int argc, char **argv) {
  int res;

  cli_t cli = parse_cli_opts(argc, argv);
  signal(SIGINT, sig_cleanup);

  dprintf(1, "watching dir: \"%s\"\n", cli.dir);
  dprintf(1, "port: \"%d\"\n", cli.port);

  fd_set fd_read;
  FD_ZERO(&fd_read);

  watch_fd = watch_dir(cli.dir);
  check(watch_fd >= 0, "failed to watch filesystem: %d", watch_fd);

  FD_SET(watch_fd, &fd_read);

  fd_set read_ready = fd_read;

  for (;;) {
    res = select(watch_fd + 1, &read_ready, NULL, NULL, NULL);
    check(res >= 0, "failed to select");

    if (FD_ISSET(watch_fd, &read_ready)) {
      watch_notifications(watch_fd);
    }

    read_ready = fd_read;
  }

  close(watch_fd);

  return 0;

error:
  return 2;
}

void sig_cleanup(int num) {
  (void)num;
  log_info("shuting down");
  if (watch_fd != -1) {
    close(watch_fd);
  }

  exit(0);
}
