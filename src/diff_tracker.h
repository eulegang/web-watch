#ifndef diff_tracker_h
#define diff_tracker_h

#include "digest.h"

#define DIFF_LIST_ITEM_SIZE 32

typedef struct {
  char *path;
  digest_t digest;
} diff_item_t;

typedef struct diff_list_t{
  diff_item_t items[32];
  struct diff_list_t *next;
} diff_list_t;

diff_list_t *diff_list_mk();
void diff_list_free(diff_list_t *);

int diff_list_change(diff_list_t*, char*);

void diff_list_display(diff_list_t*, int);

#endif
