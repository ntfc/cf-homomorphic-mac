#pragma once

#include "arith.h"
#include "msglbl.h"

#ifdef __cplusplus
extern "C" {
#endif

#define       cf_program_n_inputs(p)        (p->n_inputs)
#define       cf_program_input_lbls(p)      (p->input_lbls)
#define       cf_program_arith_circuit(p)   (p->arith)


//! @brief Type of a labeled program.
typedef struct cf_program_s *cf_program_t;

struct cf_program_s {
  size_t n_inputs;              // number of inputs labels
  cf_label_t *input_lbls; // label's vector of length n_inputs
  arith_circ_t arith;           // arith->n_inputs == n_inputs
};

void          cf_program_init(cf_program_t *, cf_label_t *, size_t,
                              const arith_circ_t);
void          cf_program_free(cf_program_t);

#ifdef __cplusplus
}
#endif
