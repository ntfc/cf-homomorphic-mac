#include "operations.h"

/**
 * Given two tags of the form \f$(y_0, \dotsc, y_d)\f$, where each \f$y_i\f$ is
 * a coefficient of polynomial (represented as a big number) and \f$d\f$ is the
 * degree of the tag, this function performs the addition of two tags.
 * If a modulo value is passed, i.e, different than \c NULL, then modulo
 * reduction is performed.
 * 
 * @param[in] a   First tag
 * @param[in] b   Second tag
 * @param[in] mod Reduction modulo mod.
 * 
 * @returns A newly allocated tag containg \f$a + b\f$ or \f$a + b \pmod{mod}\f$
 * if \p mod is not \c NULL.
 */
void tag_addition(cf_tag_t *dest, const cf_tag_t a, const cf_tag_t b, const cf_mpi_t mod) {
  assert(a && b);
  size_t d1, d2, i;
  cf_tag_t tag_a = a, tag_b = b;
  
  if(a->n < b->n) { // swap a and b
    tag_a = b;
    tag_b = a;
  }
  d1 = CF_TAG_DEGREE(tag_a);
  d2 = CF_TAG_DEGREE(tag_b);
  cf_tag_new(dest, d1);
  
  for(i = 0; i <= d1; i++) {
    if(i > d2) {
      // no addition requires, just copy the value of y1 to resulting tag
      cf_mpi_copy(&((*dest)->y[i]), tag_a->y[i]);
    }
    else {
      cf_mpi_init(&((*dest)->y[i]));
      if(mod != NULL) {
        cf_mpi_addm(&((*dest)->y[i]), tag_a->y[i], tag_b->y[i], mod);
      }
      else {
        cf_mpi_add(&((*dest)->y[i]), tag_a->y[i], tag_b->y[i]);
      }
    }
  }
}
