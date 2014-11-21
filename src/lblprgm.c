#include "lblprgm.h"

void cf_program_init(cf_program_t *prg, cf_label_t *tau, size_t n, const arith_circ_t f) {
  assert(tau);
  assert(f);
  assert(n == f->n_inputs);
  *prg = calloc(1, sizeof(struct cf_program_s));
  (*prg)->n_inputs = n;
  (*prg)->input_lbls = tau;
  (*prg)->arith = f;

}

void cf_program_free(cf_program_t prg) {
  if(prg != NULL) {
    free(prg);
    prg = NULL;
  }
}
