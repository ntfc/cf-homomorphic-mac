#include <stdio.h>
#include "cf-random.h"
#include "operations.h"
#include "test.h"
#include "timemeasure.h"
#include "keys.h"

/**
 * Generates random tags, and compares the times of two multiplication
 */
cf_errno test_tag_mul(test_params_t *params) {
  cf_errno cf_err = E_NOERR;
  cf_tag_t tag1 = NULL, tag2 = NULL;
  cf_tag_t result_fft = NULL;
  cf_tag_t result_no_fft = NULL;
  size_t i, n;
  //size_t tag_size[13] = {2, 10, 50, 100, 150, 200, 500, 750, 900, 1000, 2500, 5000, 10000};
  //size_t ntags1, ntags2;
  size_t ntags;
  cf_keys_t keys = NULL;
  cf_randstate_t prng;
  FILE *outfile = NULL;

  if(params->outfile != NULL) {
    outfile = fopen(params->outfile, "w");
    if(outfile == NULL) {
      LOG_PRINT("error opening out file. aborting\n");
      return E_ERRNO_ERROR;
    }
  }
  
  if( (cf_err = cf_mpi_library_init(&prng)) != E_NOERR)
    return cf_err;
  if( (cf_err = cf_key_gen(&keys, params->nbits, 0, CF_1, prng)) != E_NOERR)
    return cf_err;
 
  // version1
//  for(n = 0; n < 13; n++) {
//    ntags1 = tag_size[n];
//    ntags2 = tag_size[n];
//    cf_tag_random(&tag1, ntags1, keys->ek->p, prng);
//    cf_tag_random(&tag2, ntags2, keys->ek->p, prng);
//    fprintf(stderr, "tags size of %zu and %zu... ", ntags1, ntags2);
//    fprintf(stderr, "FFT... ");
//    START_MEASURE(fft);
//    tag_mul_fft(&result_fft, tag1, tag2, keys->ek->p);
//    END_MEASURE(fft);
//    fprintf(stderr, "\"bruteforce\"... ");
//    START_MEASURE(no_fft);
//    tag_mul_naive(&result_no_fft, tag1, tag2, keys->ek->p);
//    END_MEASURE(no_fft);
//    
//    assert(result_fft && result_no_fft);
//    assert(CF_TAG_DEGREE(result_fft) == CF_TAG_DEGREE(result_no_fft));
//    
//    // verify results
//    for(i = 0; i <= CF_TAG_DEGREE(result_fft); i++) {
//      if(cf_mpi_cmp(result_fft->y[i], result_no_fft->y[i]) != 0) {
//        cf_err = E_RANGE;
//      }
//    }
//    cf_tag_free(tag1);
//    cf_tag_free(tag2);
//    cf_tag_free(result_fft);
//    cf_tag_free(result_no_fft);
//    if(cf_err == E_NOERR)
//      fprintf(stderr, "=> OK\n");
//    else
//      fprintf(stderr, "=> FAIL\n");
//
//    // print times
//    PRINT_MEASURE("  with FFT", fft);
//    PRINT_MEASURE("  without FFT", no_fft);
//  }
  // version 2
  if(outfile) {
    fprintf(outfile, "# %u bits. times in ms\n", params->nbits);
    fprintf(outfile, "n\tfft\tclassical\n");
  }
  for(n = 1; n <= 100; n++) {
    ntags = n * 25;
    fprintf(stderr, "tag size of and %zu, %u bits\n", ntags, params->nbits);
    cf_tag_random(&tag1, ntags, keys->ek->p, prng);
    cf_tag_random(&tag2, ntags, keys->ek->p, prng);
    START_MEASURE(fft);
    tag_mul_fft(&result_fft, tag1, tag2, keys->ek->p);
    END_MEASURE(fft);
    PRINT_MEASURE("  with FFT", fft);

    START_MEASURE(no_fft);
    tag_mul_naive(&result_no_fft, tag1, tag2, keys->ek->p);
    END_MEASURE(no_fft);
    PRINT_MEASURE("  without FFT", no_fft);

    if(outfile) {
      fprintf(outfile, "%zu\t", ntags);
      FPRINT_WALL_TIME(outfile, fft);
      fprintf(outfile, "\t");
      FPRINT_WALL_TIME(outfile, no_fft);
      fprintf(outfile, "\n");
    }

    assert(result_fft && result_no_fft);
    assert(CF_TAG_DEGREE(result_fft) == CF_TAG_DEGREE(result_no_fft));
    // verify results
    for(i = 0; i <= CF_TAG_DEGREE(result_fft); i++) {
      if(cf_mpi_cmp(result_fft->y[i], result_no_fft->y[i]) != 0) {
        cf_err = E_RANGE;
      }
    }

    assert(cf_err == E_NOERR);

    cf_tag_free(tag1);
    cf_tag_free(tag2);
    cf_tag_free(result_fft);
    cf_tag_free(result_no_fft);
  }
  if(outfile)
    fclose(outfile);
  cf_keys_free(keys);
  cf_mpi_library_free(prng);
  return cf_err;
}
