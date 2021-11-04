#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "watch_list.h"

#define ITER_CHUNKS(list) for (;list->next;list = list->next)
#define ITER_ITEMS(list) watch_item_t _; int i; \
  for (i = 0, _ = (list)->items[i]; i < WATCH_LIST_ELEM_SIZE; _ = (list)->items[++i])


void watch_list_add(watch_list_t* list, watch_item_t item) {
  ITER_CHUNKS(list) {
    ITER_ITEMS(list) {
      if (!_.path) {
        list->items[i] = item;
        return;
      }
    }
  }

  list->next = watch_list_mk();
  list->items[0] = item;
}

void watch_list_watching(watch_list_t* list, int wd, char* path) {
  size_t len = strlen(path);

  char* owned = malloc(len * sizeof(char));
  owned[0] = 0;

  strncpy(owned, path, len);

  watch_item_t item = { .wd = wd, .path = owned };

  watch_list_add(list, item);
}

char *watch_list_lookup(watch_list_t *list, int wd) {
  ITER_CHUNKS(list) {
    ITER_ITEMS(list) {
      if (_.wd == wd) {
        return _.path;
      }
    }
  }

  return NULL;
}

void watch_list_free(watch_list_t* list) {
  if (!list) return;

  if (list->next)
    watch_list_free(list->next);

  ITER_ITEMS(list) {
    if (_.path) free(_.path);
  }

  free(list);
}

watch_list_t* watch_list_mk(void) {
  watch_list_t *list = (watch_list_t*) malloc(sizeof(watch_list_t));

  ITER_ITEMS(list) {
    list->items[i].path = NULL;
    list->items[i].wd = 0;
  }

  return list;
}

void watch_list_display(int fd, watch_list_t* list) {
  ITER_CHUNKS(list) {
    ITER_ITEMS(list) {
      if (_.path) {
        dprintf(fd, "(%d): %s\n", _.wd, _.path);
      }
    }
  }
}

int watch_list_unwatch(watch_list_t* list, int wd) {
  ITER_CHUNKS(list) {
    ITER_ITEMS(list) {
      if (_.wd == wd) {
        list->items[i].wd = 0;

        if (_.path) 
          free(_.path);

        list->items[i].path = NULL;
      }
    }
  }

  return 0;
}

int watch_list_untrack(watch_list_t* list, char *path) {
  ITER_CHUNKS(list) {
    ITER_ITEMS(list) {
      if (!strcmp(_.path, path)) {
        list->items[i].wd = 0;

        if (_.path) 
          free(_.path);

        list->items[i].path = NULL;
      }
    }
  }

  return 0;
}
