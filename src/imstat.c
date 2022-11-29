#include <fitsio.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <float.h>
#include <math.h>

typedef struct {
  float *data;
  long naxes[4];
} fits_data_t;

fits_data_t* fits_data_create() {
  fits_data_t *self = malloc(sizeof(fits_data_t));
  return self;
}

void fits_data_delete(fits_data_t *self) {
  if (self->data) {
    free(self->data);
  }
  free(self);
}

int extract_data_from_fits(fitsfile *fptr, fits_data_t *fits) {
  int hdutype, naxis;
  int status = 0;
  long *naxes = fits->naxes;
  long fpixel[4] = {1, 1, 1, 1};
  int bitpix;

  if (fits_get_hdu_type(fptr, &hdutype, &status) || hdutype != IMAGE_HDU) {
    fprintf(stderr, "Error: this program only works on images, not tables\n");
    abort();
  }
  if (status) {
    fits_report_error(stderr, status);
    abort();
  }

  fits_get_img_param(fptr, 4, &bitpix, &naxis, naxes, &status);
  if (status) {
    fits_report_error(stderr, status);
    abort();
  }
  if (naxis != 4) {
    fprintf(stderr, "Error: NAXIS = %d.  Only 4-D images are supported "
           "[Ra][Dec][Freq][Polarisation].\n",
           naxis);
    abort();
  }

  if (bitpix != FLOAT_IMG) {
    fprintf(stderr, "Error: Input has to be a float type image\n");
    abort();
  }

  if (naxes[2] != 1 && naxes[3] != 1) {
    fprintf(stderr, "Error: Dim = %ld x %ld x %ld x %ld. Dimension of "
           "polarisation axis has to be 1.\n",
           naxes[0], naxes[1], naxes[2], naxes[3]);
  }

  if (naxes[2] == 1 && naxes[3] != 1) {
    printf("Swapping polarisation and frequency axis\n");
    long temp = naxes[2];
    naxes[2] = naxes[3];
    naxes[3] = temp;
  }

  printf("Dim: %ld x %ld x %ld x %ld\n", naxes[0], naxes[1], naxes[2],
         naxes[3]);

  fits->data = (float *)malloc(naxes[0] * naxes[1] * naxes[2] * sizeof(float));

  if (fits->data == NULL) {
    fprintf(stderr, "Memory allocation error\n");
    abort();
  }

  size_t spat_size = naxes[0] * naxes[1];
  fits_read_pix(fptr, TFLOAT, fpixel, spat_size * naxes[2], 0, fits->data, 0,
                &status);

  return status;
}

int main(int argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr, "Usage: ./imstat <filename>\n");
    return 1;
  }

  int status = 0;
  fitsfile *fptr;
  printf("Reading file %s\n", argv[1]);
  fits_open_file(&fptr, argv[1], READONLY, &status);
  fits_data_t *data = fits_data_create();

  if (status) {
    fits_report_error(stderr, status);
    fprintf(stderr, "Error: Unable to open file\n");
    return status;
  }

  extract_data_from_fits(fptr, data);

  float min = FLT_MAX;
  float max = FLT_MIN;
  float mean = 0.0f;
  size_t total_pix = data->naxes[0] * data->naxes[1] * data->naxes[2] * data->naxes[3];
  size_t valid_pix = 0;
  for (size_t i = 0; i < total_pix; i++) {
    if (isnan(data->data[i])) {
      continue;
    }
    valid_pix++;
    if (data->data[i] < min)
      min = data->data[i];
    if (data->data[i] > max)
      max = data->data[i];
    mean += data->data[i];
  }
  mean /= valid_pix;

  printf(
    "Mean: %.5f\n"
    "Min:  %.5f\n"
    "Max:  %.5f\n",
    mean,
    min,
    max
    );

  fits_data_delete(data);

  fits_close_file(fptr, &status);
  if (status)
    fits_report_error(stderr, status);
}
