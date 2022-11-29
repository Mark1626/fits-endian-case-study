#include "fitsio.h"
#include <assert.h>
#include <fitsio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "longnam.h"

int main(int argc, char **argv) {
  if (argc < 3) {
    fprintf(stderr, "Usage: imcvt_big_to_little <infile> <outfile>\n");
    return EXIT_FAILURE;
  }

  const char *infilename, *outfilename;
  infilename = argv[1];
  outfilename = argv[2];

  fitsfile *infptr, *outfptr;
  int status = 0;
  fits_open_file(&infptr, argv[1], READONLY, &status);
  if (status) {
    fprintf(stderr, "Unable to open file %s\n", infilename);
    fits_report_error(stderr, status);
    return status;
  }

  fits_create_file(&outfptr, argv[2], &status);
  if (status) {
    fprintf(stderr, "Unable to create file %s\n", outfilename);
    fits_report_error(stderr, status);
    return status;
  }

  int hdutype, naxis;
  long naxes[4];
  int bitpix;

  if (fits_get_hdu_type(infptr, &hdutype, &status) || hdutype != IMAGE_HDU) {
    fprintf(stderr, "Error: this program only works on images, not tables\n");
    abort();
  }
  if (status) {
    fits_report_error(stderr, status);
    abort();
  }

  fits_get_img_param(infptr, 4, &bitpix, &naxis, naxes, &status);
  if (status) {
    fits_report_error(stderr, status);
    abort();
  }
  if (naxis != 4) {
    fprintf(stderr,
            "Error: NAXIS = %d.  Only 4-D images are supported "
            "[Ra][Dec][Freq][Polarisation].\n",
            naxis);
    abort();
  }

  if (bitpix != FLOAT_IMG) {
    fprintf(stderr, "Error: Input has to be a float type image\n");
    abort();
  }

  fits_copy_header(infptr, outfptr, &status);
  if (status) {
    fprintf(stderr, "Unable to copy FITS header\n");
    fits_report_error(stderr, status);
    return status;
  }

  // Read offsets from infile
  long long headaddr;
  long long databegin;
  long long dataend;

  fits_get_hduaddrll(infptr, &headaddr, &databegin, &dataend, &status);
  if (status) {
    fprintf(stderr, "Error: Unable to offset information\n");
    fits_report_error(stderr, status);
    abort();
  }
  printf("Data Start Offset: %lld\n", databegin);

  // Read datat from infile
  long fpixel[4] = {1, 1, 1, 1};

  size_t total_pix = naxes[0] * naxes[1] * naxes[2] * naxes[3];
  float *data = malloc(sizeof(float) * total_pix);
  if (data == NULL) {
    fprintf(stderr, "Error: Unable to allocate data\n");
    return EXIT_FAILURE;
  }

  fits_read_pix(infptr, TFLOAT, fpixel, total_pix, 0, data, 0, &status);
  if (status) {
    fprintf(stderr, "Error: Unable to read data\n");
    fits_report_error(stderr, status);
    return status;
  }

  fits_close_file(outfptr, &status);
  if (status) {
    fprintf(stderr, "Error: Unable to close file %s\n", outfilename);
    fits_report_error(stderr, status);
    return status;
  }

  fits_close_file(infptr, &status);
  if (status) {
    fprintf(stderr, "Error: Unable to close file %s\n", infilename);
    fits_report_error(stderr, status);
    return status;
  }

  // Write to file is done with normal IO

  int word_size = abs(bitpix) / 8;

  FILE *file = fopen(outfilename, "r+");
  if (file == NULL) {
    fprintf(stderr, "Error: Unable to open file\n");
    return status;
  }
  if (fseek(file, databegin, SEEK_SET)) {
    fprintf(stderr, "Unable to seek to position\n");
    return EXIT_FAILURE;
  }

  unsigned long bytes_written = 0;
  bytes_written = fwrite(data, word_size, total_pix, file);

  if (bytes_written != total_pix) {
    fprintf(stderr,
            "Error: Unable to write content, wrote only %lu elements when "
            "trying to write %lu elements\n",
            bytes_written, total_pix);
    return status;
  }

  fclose(file);
}
