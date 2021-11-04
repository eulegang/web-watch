#ifndef digest_h
#define digest_h

#include <stdint.h>
#include <stdalign.h>

typedef struct {
  unsigned char buf[16] __attribute__((aligned(alignof(uint64_t))));
} digest_t;

int digest_init(digest_t *, char*);
int digest_cmp(digest_t *, digest_t*);
void digest_repr(digest_t *, char buf[33]); 

#endif
