#include <sys/inotify.h>
#include <dirent.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <dbg.h>
#include <alloca.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "watch.h"

#include "watch_list.h"
#include "diff_tracker.h"

#define IN_DIR_FLAGS IN_CREATE | IN_DELETE | IN_DELETE_SELF
#define IN_REG_FLAGS IN_CLOSE_WRITE | IN_MOVE_SELF | IN_DELETE_SELF

#ifndef NDEBUG
#define debug_event(E) __debug_display_event((E))
void __debug_display_event(const struct inotify_event*);
#else
#define debug_event(E)
#endif

static watch_list_t *watch_list;
static diff_list_t *diff_list;

char cur_path[PATH_MAX];

int recur_walk(int, int, char*);

int inh_dir_new_ent(int, const struct inotify_event*);
int inh_self_rm(int, const struct inotify_event*);

int watch_dir(char *path) {
  cur_path[0] = 0;

  if (watch_list) 
    watch_list_free(watch_list);

  if (diff_list)
    diff_list_free(diff_list);

  watch_list = watch_list_mk();
  diff_list = diff_list_mk();

  strncat(cur_path, path, PATH_MAX);
  int len = strlen(cur_path);

  int inot_fd = inotify_init1(IN_NONBLOCK);
  check(inot_fd >= 0, "unable to open inotify");

  int wd = inotify_add_watch(inot_fd, path, IN_DIR_FLAGS);
  watch_list_watching(watch_list, wd, path);

  int res = recur_walk(inot_fd, len, cur_path);
  check(!res, "failed to walk dir");

  return inot_fd;
error:
  close(inot_fd);
  return -1;
}


void watch_notifications(int fd) {
  int res = 0;
  char buf[4096] __attribute__((aligned(__alignof__(struct inotify_event))));

  const struct inotify_event *event;
  ssize_t len;

  len = read(fd, buf, sizeof(buf));
  check(len >= 0, "failed to read notifications");
  
  for (char *ptr = buf; ptr < buf + len; ptr += sizeof(struct inotify_event) + event->len) {
    event = (const struct inotify_event *)ptr;
    if (event->mask & IN_IGNORED) continue;

    debug_event(event);

    if (event->mask & IN_CREATE) {
      res = inh_dir_new_ent(fd, event);
      if (res == -1) continue;
    }

    if (event->mask & (IN_MOVE_SELF | IN_DELETE_SELF )) {
      res = inh_self_rm(fd, event);
      if (res == -1) continue;
    }

    watch_list_display(1, watch_list);
    diff_list_display(diff_list, 1);
  }

  return;
error:
  return;
}

int recur_walk(int inot_fd, int len, char *path) {
  int res;
  DIR* dir = opendir(path);
  check(dir, "failed to open dir");

  struct dirent *ent;
  path[len] = '/';
  path[++len] = 0;

  watch_item_t item = { .wd = 0, .path = NULL };
  int path_len;

  while ((ent = readdir(dir))) {
    if (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..")) continue;

    switch (ent->d_type) {
      case DT_REG:
        strncpy(path + len, ent->d_name, PATH_MAX - len);

        path_len = strlen(ent->d_name) + len;
        item.path = malloc(sizeof(char) * path_len);
        item.path[0] = 0;

        strncpy(item.path, path, path_len);

        item.wd = inotify_add_watch(inot_fd, path, IN_REG_FLAGS);
        check(item.wd >= 0, "failed to watch file: %s", path);

        log_info("watching file: %d \"%s\"", item.wd, path);

        watch_list_add(watch_list, item);

        break;

      case DT_DIR:
        strncpy(path + len, ent->d_name, PATH_MAX - len);

        path_len = strlen(ent->d_name) + len;
        item.path = malloc(sizeof(char) * path_len);
        item.path[0] = 0;

        strncpy(item.path, path, path_len);

        item.wd = inotify_add_watch(inot_fd, path, IN_DIR_FLAGS);
        check(item.wd >= 0,  "failed to watch directory: %s", path);

        log_info("watching directory: %d \"%s\"", item.wd, path);

        watch_list_add(watch_list, item);

        res = recur_walk(inot_fd, len + strlen(ent->d_name), path);
        check(!res, "failed to recurs");

        break;

      default:
        break;
    }
  }

  res = closedir(dir);
  check(!res, "failed to close dir");

  return 0;
error:
  return -1;
  
}

int inh_dir_new_ent(int inot_fd, const struct inotify_event *event) {
  int res = 0;
  char *dir_path = watch_list_lookup(watch_list, event->wd);
  if (!dir_path) goto error;

  size_t len = strlen(event->name) + 1 + strlen(dir_path) + 1;
  char *path = malloc(len * sizeof(char));
  snprintf(path, len, "%s/%s", dir_path, event->name);

  struct stat st;
  res = stat(path, &st);
  if (res == -1) goto error;

  int flags = S_ISDIR(st.st_mode) ? 
    IN_DIR_FLAGS : 
    IN_REG_FLAGS;

  if (!S_ISDIR(st.st_mode)) {
    diff_list_change(diff_list, path);
  }

  int wd = inotify_add_watch(inot_fd, path, flags);
  if (wd == -1) goto error;

  watch_list_watching(watch_list, wd, path);

  return 0;
error:
  return -1;
}

int inh_self_rm(int inot_fd, const struct inotify_event* event) {
  (void)inot_fd;
  debug("unwatching %d", event->wd);

  watch_list_unwatch(watch_list, event->wd);

  // check if path still exists and readd is it does

  return 0;
}

#ifndef NDEBUG
void __debug_display_event(const struct inotify_event *event) {
  char *out = alloca(4096);
  char *buf = out;

  buf[0] = 0;

  buf += snprintf(buf, 4096, "wd = %d(%s)", event->wd, watch_list_lookup(watch_list, event->wd));
  buf += snprintf(buf, 4096, " mask =");

  if (event->mask & IN_ACCESS) buf += snprintf(buf, 4096, " IN_ACCESS");
  if (event->mask & IN_MODIFY) buf += snprintf(buf, 4096, " IN_MODIFY");
  if (event->mask & IN_ATTRIB) buf += snprintf(buf, 4096, " IN_ATTRIB");
  if (event->mask & IN_CLOSE_WRITE) buf += snprintf(buf, 4096, " IN_CLOSE_WRITE");
  if (event->mask & IN_CLOSE_NOWRITE) buf += snprintf(buf, 4096, " IN_CLOSE_NOWRITE");
  if (event->mask & IN_OPEN) buf += snprintf(buf, 4096, " IN_OPEN");
  if (event->mask & IN_MOVED_FROM) buf += snprintf(buf, 4096, " IN_MOVED_FROM");
  if (event->mask & IN_MOVED_TO) buf += snprintf(buf, 4096, " IN_MOVED_TO");
  if (event->mask & IN_CREATE) buf += snprintf(buf, 4096, " IN_CREATE");
  if (event->mask & IN_DELETE) buf += snprintf(buf, 4096, " IN_DELETE");
  if (event->mask & IN_DELETE_SELF) buf += snprintf(buf, 4096, " IN_DELETE_SELF");
  if (event->mask & IN_MOVE_SELF) buf += snprintf(buf, 4096, " IN_MOVE_SELF");

  if (event->len > 0) {
    buf += snprintf(buf, 4096, " name = %.*s", event->len, event->name);
  }

  debug("notify event: %s", out);
}
#endif
