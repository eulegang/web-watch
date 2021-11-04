#include <openssl/md5.h>
#include <alloca.h>
#include <dbg.h>
#include <fcntl.h>
#include <unistd.h>

#include "digest.h"

int digest_init(digest_t *digest, char* path) {
  int md_status;
  MD5_CTX ctx;
  char *buf = alloca(4096);

  md_status = MD5_Init(&ctx);
  check(md_status, "failed to init md5 context");

  int fd = open(path, O_RDONLY);
  check(fd >= 0, "failed to open file: %s", path);

  int bytes = 0;

  do {
    bytes = read(fd, buf, 4096);
    check(bytes >= 0, "failed to read from file: %s", path);
    md_status = MD5_Update(&ctx, buf, bytes);
    check(md_status, "failed to update md5 context");
  } while (bytes > 0);

  md_status = MD5_Final(digest->buf, &ctx);
  check(md_status, "failed to finalize md5 context");

  return 0;
error:
  return -1;
}

int digest_cmp(digest_t *a, digest_t *b) {
  uint64_t * ab = (uint64_t*) a->buf;
  uint64_t * bb = (uint64_t*) b->buf;

  int res = 0;

  res |= ab[0] ^ bb[0];
  res |= ab[1] ^ bb[1];
  res |= ab[2] ^ bb[2];
  res |= ab[3] ^ bb[3];

  return res;
}

void digest_repr(digest_t *digest, char buf[33]) {
  static char map[16] = {
    '0', '1', '2', '3', '4', '5', '6', '7', 
    '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
  };

  buf[0] = map[digest->buf[0] >> 4];
  buf[1] = map[digest->buf[0] & 0xF];
  buf[2] = map[digest->buf[1] >> 4];
  buf[3] = map[digest->buf[1] & 0xF];

  buf[4] = map[digest->buf[2] >> 4];
  buf[5] = map[digest->buf[2] & 0xF];
  buf[6] = map[digest->buf[3] >> 4];
  buf[7] = map[digest->buf[3] & 0xF];

  buf[8] = map[digest->buf[4] >> 4];
  buf[9] = map[digest->buf[4] & 0xF];
  buf[10] = map[digest->buf[5] >> 4];
  buf[11] = map[digest->buf[5] & 0xF];

  buf[12] = map[digest->buf[6] >> 4];
  buf[13] = map[digest->buf[6] & 0xF];
  buf[14] = map[digest->buf[7] >> 4];
  buf[15] = map[digest->buf[7] & 0xF];


  buf[16] = map[digest->buf[8] >> 4];
  buf[17] = map[digest->buf[8] & 0xF];
  buf[18] = map[digest->buf[9] >> 4];
  buf[19] = map[digest->buf[9] & 0xF];

  buf[20] = map[digest->buf[10] >> 4];
  buf[21] = map[digest->buf[10] & 0xF];
  buf[22] = map[digest->buf[11] >> 4];
  buf[23] = map[digest->buf[11] & 0xF];

  buf[24] = map[digest->buf[12] >> 4];
  buf[25] = map[digest->buf[12] & 0xF];
  buf[26] = map[digest->buf[13] >> 4];
  buf[27] = map[digest->buf[13] & 0xF];

  buf[28] = map[digest->buf[14] >> 4];
  buf[29] = map[digest->buf[14] & 0xF];
  buf[30] = map[digest->buf[15] >> 4];
  buf[31] = map[digest->buf[15] & 0xF];

  buf[32] = 0;
}
