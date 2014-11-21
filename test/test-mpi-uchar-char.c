/**
 *
 * Description of test: TODO
 *
 */
#include <stdio.h>
#include "support.h"
#include "test.h"
#include "timemeasure.h"

cf_errno convert_uchar_to_uchar(uchar **dest, size_t *n_written, const uchar *orig, size_t orig_len) {
  cf_errno cf_err = E_NOERR;
  cf_mpi_t mpi;
  char *str = NULL;
  // convert orig to mpi
  cf_err = cf_mpi_import_uchar(&mpi, orig, orig_len);
  //cf_err = uchar_to_mpi(&mpi, n_written, orig, orig_len);
  if(cf_err != E_NOERR) {
    return cf_err;
  }
  // convert mpi to char
  //cf_err = mpi_to_char(&str, n_written, mpi);
  cf_err = cf_mpi_export_char(&str, mpi);
  cf_mpi_free(mpi);
  if(cf_err != E_NOERR) {
    return cf_err;
  }
  // convert char to mpi
  //cf_err = char_to_mpi(&mpi, n_written, str);
  cf_err = cf_mpi_import_char(&mpi, str);
  free(str);
  if(cf_err != E_NOERR) {
    return cf_err;
  }
  // convert mpi to uchar
  //cf_err = mpi_to_uchar(dest, n_written, mpi);
  cf_err = cf_mpi_export_uchar(dest, n_written, mpi);
  cf_mpi_free(mpi);
  if(cf_err != E_NOERR) {
    return cf_err;
  }
  return cf_err;
}

cf_errno convert_mpi_to_mpi(cf_mpi_t *dest, const cf_mpi_t orig) {
  cf_errno cf_err = E_NOERR;
  char *str = NULL;
  uchar *ustr = NULL;
  cf_mpi_t middle = NULL;
  size_t n;
  // convert mpi to char
  //cf_err = mpi_to_char(&str, &n, orig);
  cf_err = cf_mpi_export_char(&str, orig);
  if(cf_err != E_NOERR) {
    return cf_err;
  }
  // convert char to mpi
  //cf_err = char_to_mpi(&middle, &n, str);
  cf_err = cf_mpi_import_char(&middle, str);
  free(str);
  if(cf_err != E_NOERR) {
    return cf_err;
  }
  // convert mpi to uchar
  //cf_err = mpi_to_uchar(&ustr, &n, middle);
  cf_err = cf_mpi_export_uchar(&ustr, &n, middle);
  cf_mpi_free(middle);
  if(cf_err != E_NOERR) {
    return cf_err;
  }
  // convert uchar to mpi
  //cf_err = uchar_to_mpi(dest, &n, ustr, n);
  cf_err = cf_mpi_import_uchar(dest, ustr, n);
  if(ustr != NULL)
    free(ustr);
  if(cf_err != E_NOERR) {
    *dest = NULL;
    return cf_err;
  }
  return cf_err;
}

cf_errno test_mpi_uchar_char(test_params_t *params) {
  cf_errno cf_err = E_NOERR;
  cf_randstate_t prng = NULL;
  if(params->time_measure)
    TIME_MEASURE_ON;
  
  START_MEASURE(test1);
  // TEST NR 1
  // Starting with 4 defined uchar's, convert to mpi, then to char, then to mpi
  // and then to uchar again. compare the original uchar's with the last one's
  uchar val1[] = { 0x01 };
  uchar val2[] = { 0x0A, 0x74, 0x00, 0xFF, 0x69 };
  uchar val3[] = { 0x23, 0xE5, 0xA0, 0xE0, 0xC7, 0xC4, 0x65, 0xAC, 0xFD, 0x08, 0xB9, 0x7B, 0x62, 0x6F, 0xE9, 0x9A };
  uchar val4[] = { 0xb4, 0x66, 0x5b, 0xd4, 0x24, 0x2f, 0xf2, 0x68,
    0x38, 0x89, 0xdc, 0xa8, 0x8f, 0x56, 0xad, 0xcb, 0x44, 0x7e, 0x81, 0x0c,
    0xeb, 0xfb, 0xa5, 0x6e, 0xea, 0x6b, 0x18, 0xfe, 0xc8, 0x10, 0x00};
  uchar val5[] = {0x07, 0x66, 0xd7, 0xde, 0x93, 0x5d, 0x7a, 0x16, 0x10,
    0x4e, 0xbb, 0x3c, 0xc8, 0x04, 0xda, 0x70, 0x0b, 0xe0, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0b, 0x1C, 0x87, 0x84,
    0xb4, 0x6f, 0x35, 0xa3};
  uchar *orig_values[] = { val1, val2, val3, val4, val5 };
  size_t orig_lens[] = {1, 5, 16, 31, 73 };
  size_t len;
  int i, n_values = 5, j;
  size_t n;
  uchar *ustr = NULL;
  for(i = 0; i < n_values && cf_err == E_NOERR; i++) {
    // convert orig_values[i] to MPI
    cf_err = convert_uchar_to_uchar(&ustr, &n, orig_values[i], orig_lens[i]);
    if(cf_err == E_NOERR) { // conversion went fine
      if(n != orig_lens[i]) { // but the number of bytes read is different than expected
        fprintf(stderr, "Test 1, expected to read %zu bytes, but got %zu bytes\n", orig_lens[i], n);
        cf_err = E_TEST_ERROR;
      }
      else { // the number of bytes read is the expected
        for(len = 0; len < orig_lens[i]; len++) { // compare byte by byte
          if(orig_values[i][len] != ustr[len]) {
            fprintf(stderr, "Test 1, expected to have value %.2x in position %d, but got %.2x instead\n",
                    orig_values[i][len], i, ustr[len]);
            cf_err = E_TEST_ERROR;
          }
        }
      }
      if(ustr != NULL)
        free(ustr);
    }
  }
  END_MEASURE(test1);
  PRINT_MEASURE("test1", test1);
    
  if(cf_err != E_NOERR) {
    // no cleanup necessary, so we can return error here
    return cf_err;
  }
  if( (cf_err = cf_mpi_library_init(&prng)) != E_NOERR)
    return cf_err;
  
  START_MEASURE(test2);
  // TEST NR 2
  // Generate 5 random MPIs, and then convert them to uchar, then to mpi, then
  // to char and then to MPI. compare the original MPI's with the last ones
  cf_mpi_t rand_mpi = NULL, test_mpi = NULL;
  int n_mpis = 5;
  uint sec_params[] = {64, 128, 192, 256, 512, 1024, 2048, 4096, 5000, 10000};
  int n_sec_params = 10;

  for(i = 0; i < n_sec_params && cf_err == E_NOERR; i++) {
    // generate 5 random MPIs, and check that everything went ok
    for(j = 0; j < n_mpis && cf_err == E_NOERR; j++) {
      /*rand_mpi = gcry_mpi_new(sec_params[i]);
      gcry_mpi_randomize(rand_mpi, sec_params[i], GCRY_STRONG_RANDOM);*/
      cf_mpi_random_nbits(&rand_mpi, sec_params[i], prng);
      // convert back and forth
      cf_err = convert_mpi_to_mpi(&test_mpi, rand_mpi);
      if(cf_err == E_NOERR) {
        if(cf_mpi_cmp(rand_mpi, test_mpi) != 0) {
          fprintf(stderr, "Test 2 with %u bits, expected MPI with value ", sec_params[i]);
          cf_mpi_dump(rand_mpi);
          fprintf(stderr, ", but got ");
          cf_mpi_dump(test_mpi);
          fprintf(stderr, " instead.\n");
          cf_err = E_TEST_ERROR;
        }
      }
      cf_mpi_free(rand_mpi);
      cf_mpi_free(test_mpi);
    }
  }
  END_MEASURE(test2);
  PRINT_MEASURE("test2", test2);
  assert(cf_err == E_NOERR);

  cf_mpi_library_free(prng);
  return cf_err;
}
