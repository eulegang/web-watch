#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>

#include <dbg.h>

#include "web.h"

int setup_web(int port) {
  int res = 0;
  int sock = -1;

  log_info("creating socket");

  sock = socket(AF_INET, SOCK_STREAM, 0);
  check(sock >= 0, "failed to get socket");

  log_info("binding to local address");

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = inet_addr("0.0.0.0");
  memset(addr.sin_zero, 0, sizeof addr.sin_zero);

  int flags = fcntl(sock, F_GETFL, 0);
  if (flags == -1) goto error;
  fcntl(sock, F_SETFL, flags | O_NONBLOCK);

  res = bind(sock, (struct sockaddr *)&addr, sizeof addr);
  check(!res, "failed to bind to address");

  log_info("listening");

  res = listen(sock, 10);
  check(!res, "failed to listen to socket");

  return sock;
error:
  return -1;
}

