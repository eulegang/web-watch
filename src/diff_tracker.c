#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <openssl/md5.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <alloca.h>
#include <dbg.h>

#include "diff_tracker.h"

#define ITER_CHUNKS(list) for (;list->next;list = list->next)
#define ITER_ITEMS(list) diff_item_t _; int i; \
  for (i = 0, _ = (list)->items[i]; i < DIFF_LIST_ITEM_SIZE; _ = (list)->items[++i])

diff_list_t *diff_list_mk() {
  diff_list_t *self = (diff_list_t *) malloc(sizeof(diff_list_t));

  memset(self, 0, sizeof(diff_list_t));

  return self;
}

void diff_list_free(diff_list_t *self) {
  if (!self)
    return;
 
  ITER_ITEMS(self) {
    if (_.path) 
      free(_.path);
  }

  diff_list_free(self->next);
  free(self);
}

int diff_list_change(diff_list_t *self, char *path) {
  int res = 0;
  digest_t digest;

  res = digest_init(&digest, path);
  check(!res, "failed to digest file");

  ITER_CHUNKS(self) {
    ITER_ITEMS(self) {
      if (!strcmp(_.path, path)) {
        return !digest_cmp(&digest, &_.digest);
      }
    }
  }

  return 0;
error:
  return -1;
}

void diff_list_display(diff_list_t *self, int fd) {
  ITER_CHUNKS(self) {
    ITER_ITEMS(self) {
      if (_.path) {
        char md5[33];
        digest_repr(&_.digest, md5);

        dprintf(fd, "%s = %s\n", _.path, md5);
      }
    }
  }
}

