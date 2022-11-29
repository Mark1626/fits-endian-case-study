#include "common.h"

fits_data_t *fits_data_create() {
  fits_data_t *self = malloc(sizeof(fits_data_t));
  return self;
}

void fits_data_delete(fits_data_t *self) {
  if (self->data) {
    free(self->data);
  }
  free(self);
}
