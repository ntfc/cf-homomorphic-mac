#include "arith.h"

gate_t gate_create(type_e type, edge_id_t *input_ids, size_t n_inputs, cf_mpi_t const_val) {
  gate_t g = (gate_t)calloc(1, sizeof(struct gate_s));
  edge_id_t i;
  
  g->type = type;
  g->const_val = NULL;

  if(input_ids == NULL || n_inputs == 0) {
    g->input_ids = NULL;
    g->n_inputs = 0;
  }
  else {
    g->input_ids = calloc(n_inputs, sizeof(edge_id_t));
    for(i = 0; i < n_inputs; i++) {
      g->input_ids[i] = input_ids[i];
    }
    g->n_inputs = n_inputs;
  }
  if(type == CONSTMUL) {
    //g->const_val = gcry_mpi_copy(const_val);
    cf_mpi_copy(&(g->const_val), const_val);
  }
  return g;
}

void gate_copy(const gate_t orig, gate_t *dest) {
  *dest = gate_create(orig->type, orig->input_ids, orig->n_inputs, orig->const_val);
}

void gate_free(gate_t g) {
  if(g != NULL) {
    if(g->input_ids != NULL) {
      free(g->input_ids);
      g->input_ids = NULL;
    }
    if(g->type == CONSTMUL) {
      cf_mpi_free(g->const_val);
    }
    free(g);
    g = NULL;
  }
}
