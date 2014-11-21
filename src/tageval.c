#include <stdio.h> // debugging
#include "cf.h"
#include "logger.h"
#include "operations.h"
#include "timemeasure.h"

static inline cf_tag_t _get_input1_gate(gate_t gate, size_t n_inputs,
                                        const cf_tag_t *inputs, const cf_tag_t *evals) {
  if(gate->n_inputs > 0) { // gate_id is not an input gate, it has at least 1 input
    if(gate->input_ids[0] < n_inputs) // input1 is an input gate
      return inputs[gate->input_ids[0]];
    else // input1 is a result from a previous gate evaluation, stored in evals
      return evals[gate->input_ids[0] - n_inputs];
  }
  return NULL;
}

static inline cf_tag_t _get_input2_gate(gate_t gate, size_t n_inputs,
                                        const cf_tag_t *inputs, const cf_tag_t *evals) {
  if(gate->n_inputs  == 2) { // gate_id has a 2nd input gate, so let's get it
    if(gate->input_ids[1] < n_inputs) // input2 is an input gate
      return inputs[gate->input_ids[1]];
    else // input2 is a result from a previous gate evaluation, stored in evals
      return evals[gate->input_ids[1] - n_inputs];
  }
  return NULL;
}

cf_errno _cf_eval_tags(cf_tag_t *result, const arith_circ_t a,  const cf_tag_t *tags, size_t n_inputs, const cf_mpi_t mod, cf_scheme_e cf) {
  size_t i;
  gate_id_t gate_id;
  gate_t gate = NULL;
  cf_errno cf_err = E_NOERR;
  cf_tag_t input1 = NULL, input2 = NULL;
  
  if(a == NULL) return E_NO_ARITH;
  if(n_inputs != arith_circ_n_inputs(a)) return E_INVAL_N_INPUTS;
  // no more than 2^16 dependencies for each gate..
  uint16_t out_deps_not_evaled[arith_circ_n_internal_gates(a)];
  cf_tag_t evals[arith_circ_n_internal_gates(a)];
  
  if(a->t_sort == NULL) {
    cf_err = arith_circ_topology_sort(a);
    if(cf_err != E_NOERR) return cf_err;
  }
  START_MEASURE(eval_init);
  // initialization..
  for(i = 0; i < arith_circ_n_internal_gates(a); i++) {
    // get number of out deps of gate (i + g->n_inputs)
    out_deps_not_evaled[i] = graph_al_get_gate_n_out_deps(a->graph, i + arith_circ_n_inputs(a));
    // null tags
    evals[i + arith_circ_n_inputs(a)] = NULL;
  }
  if(is_verbose) {
    END_MEASURE(eval_init);
    LOG_PRINT("[tageval] ");
    PRINT_MEASURE("eval init", eval_init);
  }

  START_MEASURE(gate_eval);
  // traverse the graph using the topological sort order
  for(gate_id = 0, i = 0; i < arith_circ_n_gates(a); i++, gate_id = a->t_sort[i]) {
    gate = a->gates[gate_id];
    
    input1 = _get_input1_gate(gate, n_inputs, tags, evals);
    input2 = _get_input2_gate(gate, n_inputs, tags, evals);
    
    switch(gate->type) {
      case INPUT:
        // Nothing to do, just validations?
        break;
      case ONEINPUT:
        // Nothing to do, just validations?
        if(cf_mpi_cmp_ui(tags[gate_id]->y[0], 0x1) != 0) {
          LOG_PRINT("[eval] was expecting a tag of a one-input gate, but ok, carry on\n");
        }
        break;
      case ADD:
        if(input1 == NULL || input2 == NULL) {
          LOG_PRINT("[eval] error: wrong number of inputs for add at gate %zu\n", gate_id);
          goto cleanup;
        }
        tag_addition(&(evals[gate_id - n_inputs]), input1, input2, mod);
        break;
      case MUL:
        if(input1 == NULL || input2 == NULL) {
          LOG_PRINT("[eval] error: wrong number of inputs for mul at gate %zu\n", gate_id);
          goto cleanup;
        }
        tag_product(&(evals[gate_id - n_inputs]), input1, input2, mod);
        break;
      case CONSTMUL:
        if(input1 == NULL) {
          LOG_PRINT("[eval] error: wrong number of inputs for const-mul at gate %zu\n", gate_id);
          goto cleanup;
        }
        tag_constant_product(&(evals[gate_id - n_inputs]), input1, gate->const_val, mod);
        break;
      default:
        cf_err = E_INVAL_GATE_TYPE;
        LOG_PRINT("[eval] error: no gate type %d found\n", gate->type);
        goto cleanup;
        break;
    }
    
    // free previous gates that are no more necessary
    if(gate_id >= n_inputs) { // not an input gate
      if(gate->input_ids[0] >= n_inputs) { // input1 is not an input gate
        if(--out_deps_not_evaled[gate->input_ids[0] - n_inputs] == 0) {
          // it is possible to free the result of gate->input_ids[0]
          cf_tag_free(evals[gate->input_ids[0] - n_inputs]);
          evals[gate->input_ids[0] - n_inputs] = NULL;
        }
      }
      if(gate->type != CONSTMUL && gate->input_ids[1] >= n_inputs) {
        if(--out_deps_not_evaled[gate->input_ids[1] - n_inputs] == 0) {
          // it is possible to free the result gate->input_ids[1]
          cf_tag_free(evals[gate->input_ids[1] - n_inputs]);
          evals[gate->input_ids[1] - n_inputs] = NULL;
        }
      }
    }
  }
  if(is_verbose) {
    END_MEASURE(gate_eval);
    LOG_PRINT("[tageval] ");
    PRINT_MEASURE("gate eval", gate_eval);
  }
  START_MEASURE(tag_copy);
  (*result) = cf_tag_copy(evals[arith_circ_n_internal_gates(a) - 1]);
  if(is_verbose) {
    END_MEASURE(tag_copy);
    LOG_PRINT("[tageval] ");
    PRINT_MEASURE("tag copy", tag_copy);
  }
  
  if(cf == CF_2) {
    
  }

  START_MEASURE(cleanup);
cleanup:
  // free any remaining gates, except the result
  for(i = 0; i < (arith_circ_n_internal_gates(a)); i++) {
    if(evals[i] != NULL) {
      cf_tag_free(evals[i]);
      evals[i] = NULL;
    }
  }
  if(is_verbose) {
    END_MEASURE(cleanup);
    LOG_PRINT("[tageval] ");
    PRINT_MEASURE("cleanup", cleanup);
  }
  return cf_err;
}
