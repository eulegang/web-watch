#include <threads.h>
#include <alloca.h>
#include <unistd.h>
#include <dbg.h>
#include <stdbool.h>

#define THREAD_COUNT 4
#define FD_COUNT 16

int handler_main(void* arg);
int take_fd(void);
int fds_full(void);

thrd_t thrd_ids[THREAD_COUNT];
static bool shutdown_now = false;

mtx_t fd_queue_mtx;
cnd_t fd_queue_fill;

int fds[FD_COUNT];
int fds_start = 0, fds_end = 0;

void setup_handlers(void) {
  mtx_init(&fd_queue_mtx, mtx_plain);
  cnd_init(&fd_queue_fill);

  for (int i = 0; i < THREAD_COUNT; i++) {
    thrd_create(thrd_ids + i, handler_main, NULL);
  }
}

void shutdown_handlers(void) {
  mtx_lock(&fd_queue_mtx);
  shutdown_now = true;
  cnd_broadcast(&fd_queue_fill);
  mtx_unlock(&fd_queue_mtx);

  for (int i = 0; i < THREAD_COUNT; i++) {
    thrd_join(thrd_ids[i], NULL);
  }

  cnd_destroy(&fd_queue_fill);
  mtx_destroy(&fd_queue_mtx);
}

int push_socket(int fd) {
  log_info("pushing socket: %d", fd);
  if (mtx_lock(&fd_queue_mtx))
    return -1;

  if (fds_full()) {
    mtx_unlock(&fd_queue_mtx);
    return -1;
  }

  fds[fds_end] = fd;
  fds_end += 1;
  fds_end %= FD_COUNT;

  cnd_signal(&fd_queue_fill);

  mtx_unlock(&fd_queue_mtx);

  return 0;
}

int handler_main(void* arg) {
  (void) arg;
  int fd;

  log_info("Starting web worker");

reset:
  fd = take_fd();

  if (fd == -1) {
    log_info("sleeping worker");
    mtx_lock(&fd_queue_mtx);
    cnd_wait(&fd_queue_fill, &fd_queue_mtx);
    mtx_unlock(&fd_queue_mtx);
    log_info("waking up");
    goto reset;
  }

  log_info("working on request");
 
  char* buf = alloca(4096);

  int bytes_read = read(fd, buf, 4096);

  log_info("Request: %.*s", bytes_read, buf);

  const char* resp = "HTTP/1.1 404 Not Found\r\nConnection: close\r\nContent-Type: text/plain\r\n\r\nPage not found";

  write(fd, resp, strlen(resp));

  close(fd);

  goto reset;

  return 0;
}

int take_fd(void) {
  int fd;
  if (mtx_lock(&fd_queue_mtx))
    goto error;

  if (fds_start == fds_end) {
    fd = -1;
  } else {
    fd = fds[fds_start];

    fds_start += 1;
    fds_start %= FD_COUNT;
  }

  mtx_unlock(&fd_queue_mtx);

  return fd;
error:
  return -1;
}

int fds_full(void) {
  int res = fds_start - 1 == fds_end;
  res |= fds_end == FD_COUNT - 1 && fds_start == 0;

  return res;
}
