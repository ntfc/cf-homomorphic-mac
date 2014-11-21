#include <stdio.h>
#include "arith.h"
#include "support.h"
#include "pinocchio.h"
#include "logger.h"

/**
 * If the modulo parameter \p mod is \c NULL, then no modulo reduction is done.
 * 
 * @param[out] result   Pointer to a MPI where the resulting value is stored.
 * @param[in]  a        Arithmetic circuit.
 * @param[in]  input    Array of inputs.
 * @param[in]  n_inputs Length of the array of inputs.
 * @param[in]  mod      Value of the modulo.
 * 
 * @todo Finish error codes.
 * 
 * @retval #E_NOERR            Everything OK.
 * @retval #E_NO_ARITH         No arithmetic circuit, ie. \c a is \c NULL.
 * @retval #E_INVAL_N_INPUTS   Wrong number of inputs.
 * @retval #E_INVAL_GATE_TYPE  Invalid gate type.
 */
 
static inline cf_mpi_t _get_input1_gate(gate_t gate, size_t n_inputs,
                                        const cf_mpi_t *inputs, const cf_mpi_t *evals) {
  if(gate->n_inputs > 0) { // gate_id is not an input gate, it has at least 1 input
    if(gate->input_ids[0] < n_inputs) // input1 is an input gate
      return inputs[gate->input_ids[0]];
    else // input1 is a result from a previous gate evaluation, stored in evals
      return evals[gate->input_ids[0] - n_inputs];
  }
  return NULL;
}

static inline cf_mpi_t _get_input2_gate(gate_t gate, size_t n_inputs,
                                        const cf_mpi_t *inputs, const cf_mpi_t *evals) {
  if(gate->n_inputs  == 2) { // gate_id has a 2nd input gate, so let's get it
    if(gate->input_ids[1] < n_inputs) // input2 is an input gate
      return inputs[gate->input_ids[1]];
    else // input2 is a result from a previous gate evaluation, stored in evals
      return evals[gate->input_ids[1] - n_inputs];
  }
  return NULL;
}

cf_errno arith_circ_eval(cf_mpi_t *result, const arith_circ_t a,  cf_mpi_t *input, size_t n_inputs, const cf_mpi_t mod) {
  size_t i;
  gate_id_t gate_id;
  gate_t gate = NULL;
  cf_errno cf_err = E_NOERR;
  cf_mpi_t input1 = NULL, input2 = NULL;
  
  if(a == NULL) return E_NO_ARITH;
  if(n_inputs != arith_circ_n_inputs(a)) return E_INVAL_N_INPUTS;
  // no more than 2^16 dependencies for each gate..
  uint16_t out_deps_not_evaled[arith_circ_n_internal_gates(a)];
  cf_mpi_t evals[arith_circ_n_internal_gates(a)];
  
  // we assume this is already done after parsing..
  if(a->t_sort == NULL) {
    cf_err = arith_circ_topology_sort(a);
    if(cf_err != E_NOERR) return cf_err;
  }

  // initialization..
  for(i = 0; i < arith_circ_n_internal_gates(a); i++) {
    // get number of out deps of gate (i + g->n_inputs)
    out_deps_not_evaled[i] = graph_al_get_gate_n_out_deps(a->graph, (i + n_inputs));
    evals[i] = NULL;
  }

  // traverse the graph using the topological sort order
  for(gate_id = 0, i = 0; i < arith_circ_n_gates(a); i++, gate_id = a->t_sort[i]) {
    gate = a->gates[gate_id];
    
    input1 = _get_input1_gate(gate, n_inputs, input, evals);
    input2 = _get_input2_gate(gate, n_inputs, input, evals);
    
    switch(gate->type) {
      case INPUT:
        // Nothing to do, just validations?
        break;
      case ONEINPUT:
        // Nothing to do, just validations?
        if(cf_mpi_cmp_ui(input[gate_id], 0x1) != 0) {
          LOG_PRINT("[msgeval] was expecting a one-input gate, but ok, carry on\n");
        }
        break;
      case ADD:
        if(input1 == NULL || input2 == NULL) {
          LOG_PRINT("[msgeval] error: wrong number of inputs for add at gate %zu\n", gate_id);
          goto cleanup;
        }
        cf_mpi_addm(&(evals[gate_id - n_inputs]), input1, input2, mod);
        break;
      case MUL:
        if(input1 == NULL || input2 == NULL) {
          LOG_PRINT("[msgeval] error: wrong number of inputs for mul at gate %zu\n", gate_id);
          goto cleanup;
        }
        cf_mpi_mulm(&(evals[gate_id - n_inputs]), input1, input2, mod);
        break;
      case CONSTMUL:
        if(input1 == NULL) {
          LOG_PRINT("[msgeval] error: wrong number of inputs for const-mul at gate %zu\n", gate_id);
          goto cleanup;
        }
        cf_mpi_mulm(&(evals[gate_id - n_inputs]), input1, gate->const_val, mod);
        break;
      default:
        cf_err = E_INVAL_GATE_TYPE;
        LOG_PRINT("[msgeval] error: no gate type %d found\n", gate->type);
        goto cleanup;
        break;
    }
    
    // free previous gates that are no more necessary
    if(gate_id >= n_inputs) { // not an input gate
      if(gate->input_ids[0] >= n_inputs) { // input1 is not an input gate
        if(--out_deps_not_evaled[gate->input_ids[0] - n_inputs] == 0) {
          // it is possible to free the result of gate->input_ids[0]
          cf_mpi_free(evals[gate->input_ids[0] - n_inputs]);
          evals[gate->input_ids[0] - n_inputs] = NULL;
        }
      }
      if(gate->type != CONSTMUL && gate->input_ids[1] >= n_inputs) {
        if(--out_deps_not_evaled[gate->input_ids[1] - n_inputs] == 0) {
          // it is possible to free the result gate->input_ids[1]
          cf_mpi_free(evals[gate->input_ids[1] - n_inputs]);
          evals[gate->input_ids[1] - n_inputs] = NULL;
        }
      }
    }
  }
  
  cf_mpi_copy(result, evals[arith_circ_n_internal_gates(a) - 1]);

cleanup:
  // free any remaining gates, except the result
  for(i = 0; i < (arith_circ_n_internal_gates(a)); i++) {
    if(evals[i] != NULL) {
      cf_mpi_free(evals[i]);
      evals[i] = NULL;
    }
  }
  return cf_err;
}

#pragma GCC diagnostic push
//#pragma GCC diagnostic ignored "-Wunused-parameter"
/**
 * If the modulo parameter \p mod is \c NULL, then no modulo reduction is done.
 * 
 * @param[in] f     The arithmetic circuit.
 * @param[in] m     Array of messages.
 * @param[in] m_len Length of the array of messages.
 * @param[in] mod   Value of the modulo.
 * 
 * @return The evaluated message, or \c NULL if an error ocorred.
 * @todo Error codes.
 * @note
 * This looks slower than native arith_circ_eval():
 *  * degree 100 polynomial evaluation:
 *   - pinocchio: ~800ms
 *   - native: ~10ms
 *  * degree 200 polynomial evaluation:
 *   - pinocchio: ~5500ms
 *   - native: ~60ms
 *  * degree 300 polynomial evaluation:
 *   - pinocchio: ~10000ms
 *   - native: ~110ms
 */
cf_errno arith_circ_eval_pinocchio(cf_mpi_t *msg, const arith_circ_t f, cf_mpi_t *m, size_t m_len, const cf_mpi_t mod) {
  assert(f && f->fn);
  assert(m);
  cf_mpi_t *output = NULL;
  size_t output_len = 0;
  char in[17] = "/tmp/cfXXXXXX.in\0";
  char out[18] = "/tmp/cfXXXXXX.out\0";
  int in_fd = mkstemps(in, 3);
  int out_fd = mkstemps(out, 4);

  assert(in_fd != -1 && out_fd != -1);
  arith_circ_write_io(in, m, m_len);

  // create process to aritheval.py
  pid_t pid = fork();
  int status;
  if(pid == 0) {
    execl("/usr/bin/python2", "/usr/bin/python2", PINOCCHIO_AC_EVAL, f->fn, in, out, NULL);
  }
  waitpid(pid, &status, 0);
  
  output = arith_circ_parse_io(out, &output_len);
  assert(output_len == 1);

  remove(in);
  remove(out);

  if(mod != NULL) {
    cf_mpi_mod(&(output[0]), output[0], mod);
  }
  // free unused memory
  cf_mpi_copy(msg, *output);
  cf_messages_free(output, output_len);
  
  return E_NOERR;
}

#pragma GCC diagnostic pop
