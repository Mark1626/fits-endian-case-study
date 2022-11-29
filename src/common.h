#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

typedef struct {
  float *data;
  long naxes[4];
} fits_data_t;

fits_data_t *fits_data_create();
void fits_data_delete(fits_data_t *self);

inline int is_little_endian(void) {
  const unsigned int n = 1U;
  return *((unsigned char *)&n) == 1U;
}

inline void data_swap_byte_order_float(float *raw, size_t size) {
  if (is_little_endian()) {
    printf("Little endian conversion\n");
    // TODO: Use reinterpret_cast
    uint32_t *ptr = (uint32_t *)raw;

    #pragma omp parallel for schedule(static)
    for (size_t i = 0; i < size; i += 1) {
      ptr[i] = __builtin_bswap32(ptr[i]);
    }
  }

  return;
}
