#include "operations.h"
#include "fft_ntl.h"

int use_fft = 1;

void tag_product(cf_tag_t *dest, const cf_tag_t a, const cf_tag_t b, const cf_mpi_t mod) {
  if(a->n >= b->n) {
    if(use_fft == 1)
      tag_mul_fft(dest, a, b, mod);
    else
      tag_mul_naive(dest, a, b, mod);
  }
  else {
    if(use_fft == 1)
      tag_mul_fft(dest, b, a, mod);
    else
      tag_mul_naive(dest, b, a, mod);
  }
}

void tag_mul_fft(cf_tag_t *dest, const cf_tag_t a, const cf_tag_t b, const cf_mpi_t mod) {
  assert(a);
  assert(b);

  tag_fft_mul_mod(dest, a, b, mod);
}

void tag_mul_naive(cf_tag_t *dest, const cf_tag_t a, const cf_tag_t b, const cf_mpi_t mod) {
  assert(a);
  assert(b);

  size_t d1 = CF_TAG_DEGREE(a), d2 = CF_TAG_DEGREE(b);
  size_t k, i, d = d1 + d2;
  cf_tag_new(dest, d);
  cf_mpi_t y1 = NULL, y2 = NULL, tmp = NULL;

  for(k = 0; k <= d; k++) {
    cf_mpi_init(&tmp);
    cf_mpi_init(&((*dest)->y[k]));
    
    for(i = 0; i <= k; i++) {
      if(i > d1) {
        cf_mpi_init(&y1); // 0x0
      }
      else {
        cf_mpi_copy(&y1, a->y[i]);
      }

      if((k-i) > d2) {
        cf_mpi_init(&y2); // 0x0
      }
      else {
        cf_mpi_copy(&y2, b->y[k-i]);
      }
      if(mod != NULL) {
        cf_mpi_mulm(&tmp, y1, y2, mod);
        cf_mpi_addm(&((*dest)->y[k]), (*dest)->y[k], tmp, mod);
      }
      else {
        cf_mpi_mul(&tmp, y1, y2);
        cf_mpi_add(&((*dest)->y[k]), (*dest)->y[k], tmp);
      }
      
      cf_mpi_free(y1);
      cf_mpi_free(y2);
    }
    cf_mpi_free(tmp);
  }
}

void tag_constant_product(cf_tag_t *dest, const cf_tag_t a, const cf_mpi_t c, const cf_mpi_t mod) {
  size_t d1 = CF_TAG_DEGREE(a);
  cf_mpi_t zero = NULL;
  size_t i;
  cf_tag_new(dest, d1);
  cf_mpi_init(&zero);
  if(cf_mpi_cmp(zero, c) == 0) { // zero multiplication, just return tag 0
    for(i = 0; i < a->n; i++) {
      //(*dest)->y[i] = gcry_mpi_copy(zero);
      cf_mpi_copy(&((*dest)->y[i]), zero);
    }
  }
  else {
    for(i = 0; i < a->n; i++) {
      //(*dest)->y[i] = gcry_mpi_new(1);
      cf_mpi_init(&((*dest)->y[i]));
      if(mod != NULL) {
        //gcry_mpi_mulm((*dest)->y[i], a->y[i], c, mod);
        cf_mpi_mulm(&((*dest)->y[i]), a->y[i], c, mod);
      }
      else {
        //gcry_mpi_mul((*dest)->y[i], a->y[i], c);
        cf_mpi_mul(&((*dest)->y[i]), a->y[i], c);
      }
    }
  }
  //gcry_mpi_release(zero);
  cf_mpi_free(zero);
}
