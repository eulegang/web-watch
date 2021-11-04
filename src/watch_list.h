#ifndef watch_list_h
#define watch_list_h

#define WATCH_LIST_ELEM_SIZE 32

typedef struct {
  int wd;
  char *path;
} watch_item_t;

typedef struct watch_list_t {
  watch_item_t items[WATCH_LIST_ELEM_SIZE];
  struct watch_list_t *next;
} watch_list_t;

void watch_list_add(watch_list_t*, watch_item_t);
void watch_list_watching(watch_list_t*, int, char*);
char *watch_list_lookup(watch_list_t*, int);
void watch_list_free(watch_list_t*);
watch_list_t* watch_list_mk(void);
int watch_list_unwatch(watch_list_t*, int);
int watch_list_untrack(watch_list_t*, char*);
void watch_list_display(int, watch_list_t*);

#endif
