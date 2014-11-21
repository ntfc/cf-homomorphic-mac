#pragma once

#include "tag.h"

#ifdef __cplusplus
extern "C" {
#endif

void gcry_fft_mul_mod(cf_mpi_t **result, size_t *result_len, const cf_mpi_t *a,
                      size_t a_len, const cf_mpi_t *b, size_t b_len,
                      const cf_mpi_t mod);

void cf_mpi_fft_mul_mod(mpz_t **result, size_t *result_len, mpz_t *a,
                        size_t a_len, mpz_t *b, size_t b_len,
                        const mpz_t mod);
                 
void tag_fft_mul_mod(cf_tag_t *tag, const cf_tag_t a, const cf_tag_t b, const cf_mpi_t mod);

#ifdef __cplusplus
} // extern "C"
#endif
