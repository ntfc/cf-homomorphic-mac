#include <NTL/ZZ_pX.h>
#include "fft_ntl.h"

using namespace std;
using namespace NTL;

void cf_mpi_fft_mul_mod(cf_mpi_t **result, size_t *result_len, cf_mpi_t *a,
                 size_t a_len, cf_mpi_t *b, size_t b_len,
                 const cf_mpi_t mod) {
  size_t i = 0;
  long j;
  ZZ_pX poly_a, poly_b, poly_result;
  char *str = NULL;
  
  size_t n_bytes = 0;
  uchar *buf;

  assert(mod && a_len > 0 && b_len > 0);
  // init ZZ_p
  str = mpz_get_str(NULL, 10, mod->mpi);
  ZZ_p::init(conv<ZZ>(conv<ZZ>(str))); // TODO: is there a way to free ZZ_p?
  free(str); str = NULL;
  
  // convert vectors of coefficients to polynomials
  for(i = 0; i < a_len; i++) {
    str = mpz_get_str(NULL, 10, a[i]->mpi);
    ZZ_p coeff = conv<ZZ_p>(conv<ZZ>(str));
    free(str); str = NULL;
    SetCoeff(poly_a, i, coeff);
  }
  for(i = 0; i < b_len; i++) {
    str = mpz_get_str(NULL, 10, b[i]->mpi);
    ZZ_p coeff = conv<ZZ_p>(conv<ZZ>(str));
    free(str); str = NULL;
    SetCoeff(poly_b, i, coeff);
  }
  
  // Perform the FFT multiplication
  FFTMul(poly_result, poly_a, poly_b); // defined in lzz_pX.c
  //mul(poly_result, poly_a, poly_b); // defined in lzz_pX.c
  assert(deg(poly_result) >= 0);
  
  // allocate space for the result
  *result_len = deg(poly_result) + 1;
  *result = new cf_mpi_t[*result_len]();
  
  // extract the coeffients from poly_result
  ZZ_p coeff_result;
  assert(deg(poly_result) >= 0);
  for(j = 0; j <= deg(poly_result); j++) {
    coeff_result = coeff(poly_result, j);
    n_bytes = NumBytes(conv<ZZ>(coeff_result) % ZZ_p::modulus());
    assert(n_bytes > 0);
    buf = new uchar[n_bytes];
    BytesFromZZ(buf, conv<ZZ>(coeff_result), n_bytes);

    
    //mpz_import((*result)[j]->mpi, n_bytes, -1, sizeof(buf[0]), 0, 0, buf);
    cf_mpi_import(&((*result)[j]), n_bytes, -1, sizeof(buf[0]), 0, 0, buf);
    
    delete [] buf;
  }
  // cleanup
  poly_result.kill();
  poly_a.kill();
  poly_b.kill();
}


void tag_fft_mul_mod(cf_tag_t *tag, const cf_tag_t a, const cf_tag_t b, const cf_mpi_t mod) {
  // normal FFT mul
  cf_mpi_t *result = NULL;
  size_t result_len = 0, i;
  
  cf_mpi_fft_mul_mod(&result, &result_len, a->y, a->n, b->y, b->n, mod);

  // construct tag
  cf_tag_new(tag, result_len - 1);

  cf_tag_set_coeffs(*tag, result, result_len);

  // freeing this here because it doesnt seem possible to free memory allocated
  // with C++ in C.
  for(i = 0; i < result_len; i++)
    cf_mpi_free(result[i]);
  delete [] result;
}

/**
 * @brief Computes the FFT multiplication of two vectores of coefficients
 * 
 * @deprecated Use cf_mpi_fft_mul_mod instead
 */
void gcry_fft_mul_mod(gcry_mpi_t **result, size_t *result_len, const gcry_mpi_t *a,
                 size_t a_len, const gcry_mpi_t *b, size_t b_len,
                 const gcry_mpi_t mod) {
  size_t i = 0;
  long j;
  ZZ_pX poly_a, poly_b, poly_result;
  char *str = NULL;
  
  size_t n_bytes = 0;
  uchar *buf;
  mpz_t tmp;
  gcry_mpi_t coeff_final;
  
  // convert mod in gcrypt to gmp. then convert it to ZZ, and set it to ZZ_p
  mpz_t mod_gmp;
  gcrypt_to_mpz(mod_gmp, mod);
  // init ZZ_p
  str = mpz_get_str(NULL, 10, mod_gmp);
  ZZ_p::init(conv<ZZ>(conv<ZZ>(str)));
  free(str); str = NULL;
  mpz_clear(mod_gmp); // free mod gmp
  
  mpz_t coeff_i;
  
  // convert vectors of coefficients to polynomials
  for(i = 0; i < a_len; i++) {
    gcrypt_to_mpz(coeff_i, a[i]);
    str = mpz_get_str(NULL, 10, coeff_i);
    ZZ_p coeff = conv<ZZ_p>(conv<ZZ>(str));
    mpz_clear(coeff_i);
    free(str); str = NULL;
    SetCoeff(poly_a, i, coeff);
  }
  for(i = 0; i < b_len; i++) {
    gcrypt_to_mpz(coeff_i, b[i]);
    str = mpz_get_str(NULL, 10, coeff_i);
    ZZ_p coeff = conv<ZZ_p>(conv<ZZ>(str));
    mpz_clear(coeff_i);
    free(str); str = NULL;
    SetCoeff(poly_b, i, coeff);
  }
  
  // Perform the FFT multiplication
  FFTMul(poly_result, poly_a, poly_b); // defined in lzz_pX.c
  assert(deg(poly_result) >= 0);
  
  // allocate space for the result
  *result_len = deg(poly_result) + 1;
  (*result) = new gcry_mpi_t[*result_len]();
  
  // extract the coeffients from poly_result
  ZZ_p coeff_result;
  assert(deg(poly_result) >= 0);
  for(j = 0; j <= deg(poly_result); j++) {
    coeff_result = coeff(poly_result, j);
    n_bytes = NumBytes(conv<ZZ>(coeff_result) % ZZ_p::modulus());
    assert(n_bytes > 0);
    buf = new uchar[n_bytes];
    BytesFromZZ(buf, conv<ZZ>(coeff_result), n_bytes);
    // convert ZZ to mpz, and then to gcrypt. TODO: convert straight to crypt
    mpz_init(tmp);
    mpz_import(tmp, n_bytes, -1, sizeof(buf[0]), 0, 0, buf);
    mpz_to_gcrypt(&coeff_final, tmp);
    (*result)[j] = gcry_mpi_copy(coeff_final);
    mpz_clear(tmp);
    gcry_mpi_release(coeff_final);
    delete [] buf;
  }
  // cleanup
  poly_result.kill();
  poly_a.kill();
  poly_b.kill();
}

