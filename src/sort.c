#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include "rng.h"
#include "sort.h"

bool rd_sorted(void *base, size_t nmemb, size_t size,
               int (*cmp)(const void *, const void *)) {
  if (size == 0) {
    return true;
  }
  register uint8_t *a = (base), *b = (base) + size,
                   *end = (base) + (size * nmemb);
  while (b != end) {
    if (cmp((void *)a, (void *)b) > 0) {
      return false;
    }
    a = b, b += size;
  }
  return true;
}

void rd_knuth_shuffle(void *base, size_t nmemb, size_t size, rd_rng *r) {
  uint8_t swap[size];
  int x;
  for (size_t i = nmemb - 1; i > 0; --i) {
    x = rd_rng_int(r, i + 1);
    memcpy(&swap[0], (void *)((uint8_t *)base + x * size), size);
    memcpy((void *)((uint8_t *)base + (x * size)),
           (void *)((uint8_t *)base + (i * size)), size);
    memcpy((void *)((uint8_t *)base + (i * size)), &swap[0], size);
  }
}

size_t rd_bogosort(void *base, size_t nmemb, size_t size,
                   int (*cmp)(const void *, const void *), rd_rng *r) {
  size_t attempts = 0;
  while (!rd_sorted(base, nmemb, size, cmp)) {
    ++attempts;
    rd_knuth_shuffle(base, nmemb, size, r);
  }
  return attempts;
}

void rd_stdbogosort(void *base, size_t nmemb, size_t size,
                    int (*cmp)(const void *, const void *)) {
  rd_rng r;
  rd_rng_init_time(&r);
  rd_bogosort(base, nmemb, size, cmp, &r);
}

void rd_isort(void *base, size_t nmemb, size_t size,
              int (*cmp)(const void *, const void *)) {
  uint8_t swap[size]; // Would need more vars with a swap macro...
  for (int i = 1; i < nmemb; ++i) {
    memcpy(&swap[0], (void *)((uint8_t *)base + i * size), size);
    int j = i;
    for (; j > 0 && cmp(&swap, base + (j - 1) * size); --j) {
      memcpy((void *)((uint8_t *)base + j * size),
             (void *)((uint8_t *)base + (j - 1) * size), size);
    }
    memcpy((void *)((uint8_t *)base + j * size), &swap[0], size);
  }
}
