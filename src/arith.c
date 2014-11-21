#include <stdio.h>
#include "cf.h"

/**
 * @param[out] a  Pointer to an arithmetic circuit.
 */
void arith_circ_init(arith_circ_t *a) {
  (*a) = (arith_circ_t)calloc(1, sizeof(struct arith_circ_s));
  if((*a) == NULL) {
    fprintf(stderr, "error allocating cf_arith_circ_t\n");
    //return E_NO_MEM;
  }
  (*a)->n_inputs = 0;
  (*a)->n_one_inputs = 0;
  (*a)->fn = NULL;
  (*a)->graph = NULL;
  (*a)->gates = NULL;
  (*a)->t_sort = NULL;
}

/**
 * If the arithmetic circuit argument \p a is \c NULL, nothing is done and
 * everything is left untouched.
 * @param[in] a       An arithmetic circuit.
 * @param[in] n_gates Number of gates to initialize.
 */
void arith_circ_init_gates(arith_circ_t a, size_t n_gates) {
  size_t i;
  if(a != NULL) {
    a->graph = graph_create(n_gates);
    a->gates = (gate_t*)calloc(n_gates, sizeof(gate_t));
    for(i = 0; i < a->graph->n_nodes; i++) {
      a->gates[i] = NULL;
    }
  }
}
/**
 * If the arithmetic circuit argument \p a is \c NULL, nothis is done.
 * 
 * @param[in] a  An arithmetic circuit.
 * 
 * \note The arith_circ_t#graph is released in here because the upper levels
 * modules are not supposed to use the graph_* functions!
 */
void arith_circ_free(arith_circ_t a) {
  size_t i;
  if(a != NULL) {
    if(a->fn != NULL) {
      free(a->fn);
      a->fn = NULL;
    }
    if(a->gates != NULL) {
      for(i = 0; i < a->graph->n_nodes; i++) {
        gate_free(a->gates[i]);
      }
      free(a->gates);
      a->gates = NULL;
    }
    if(a->t_sort != NULL) {
      free(a->t_sort);
      a->t_sort = NULL;
    }
    if(a->graph != NULL) {
      graph_free(a->graph);
    }
    free(a);
    a = NULL;
  }
}

/**
 * If the arithmetic circuit argument \p a is \c NULL, or \p source is greater
 * or equal to #arith_circ_n_gates, or \p gate is \c NULL, then nothing is done.
 * 
 * @param[in] a       Arithmetic circuit.
 * @param[in] source  Gate ID.
 * @param[in] gate    The gate itself.
 * 
 * @retval #E_NOERR    No errors occurred.
 * @retval #E_GATE_ADD Error adding gate to arithmetic circuit, if one of the 
 * arguments is invalid.
 */
cf_errno arith_circ_add_gate(arith_circ_t a, edge_id_t source, gate_t gate) {
  if(a && a->graph && (source < a->graph->n_nodes) && gate) {
    gate_copy(gate, &(a->gates[source]));
    
    if(gate->type == INPUT || gate->type == ONEINPUT) {
      a->n_inputs++;
      a->graph->n_inputs = a->n_inputs;
    }
    if(gate->type == ONEINPUT) {
      a->n_one_inputs++;
    }
    return E_NOERR;
  }
  else
    return E_GATE_ADD;
}

/**
 * If \p a is \c NULL, nothing is done.
 * 
 * @param[in] a Arithmetic circuit.
 * 
 * @retval #E_NOERR       Everything OK.
 * @retval #E_NO_ARITH    No arithmetic circuit, ie. \c a is \c NULL.
 * @retval #E_INVAL_ARITH Not a valid arithmetic circuit.
 * @retval #E_ERRNO_ERROR Error code originated from errno.
 * 
 * @todo Refactor to properly use error codes.
 */
cf_errno arith_circ_topology_sort(arith_circ_t a) {
  size_t i;
  s_edge_t *aux = NULL;
  FILE *fp = NULL, *res_fp = NULL;
  pid_t pid;
  int status;
  char line[16];
  char id[16];
  cf_errno cf_err = E_NOERR;
  
  if(a == NULL)
    return E_NO_ARITH;
  if(a->graph == NULL)
    return E_INVAL_ARITH;
  
  char tsort_fn[17] = "/tmp/tsortXXXXXX\0";
  char res_fn[22] = "/tmp/tsortXXXXXX.sort\0";
  
  int tsort_fd = mkstemp(tsort_fn);
  int res_fd = mkstemps(res_fn, 5);
  if(tsort_fd == -1 || res_fd == -1) {
    return E_ERRNO_ERROR;
  }

  // TODO: get FILE from already open tsort_fd
  fp = fdopen(tsort_fd, "w");
  if(fp == NULL) return E_ERRNO_ERROR;
  
  for(i = 0; i < arith_circ_n_gates(a); i++) {
    aux = a->graph->adj_lists[i];
    while(aux) {
      fprintf(fp, "%zu %zu\n", i, aux->dest);
      aux = aux->next;
    }
  }
  fclose(fp);
  
  // execute tsort
  pid = fork();
  if(pid == 0) {
    dup2(res_fd, 1); // stdout to res_fd file
    execl("/usr/bin/tsort", "/usr/bin/tsort", tsort_fn, NULL);
  }
  waitpid(pid, &status, 0);
  
  close(res_fd);
  
  res_fp = fopen(res_fn, "r");
  assert(res_fp != NULL);
  
  // TODO: read string to size_t
  i = 0;
  a->t_sort = (gate_id_t*)calloc(arith_circ_n_gates(a) + 1, sizeof(gate_id_t));

  while(fgets(line, sizeof(line), res_fp) && i < arith_circ_n_gates(a)) {
    sscanf(line, "%s\n", id);
    if((cf_err = str_to_size(&(a->t_sort[i]), id, 10)) != E_NOERR)
      break;
    //a->t_sort[i] = (edge_id_t)id;
    i++;
  }
  fclose(res_fp);
  
  remove(tsort_fn);
  remove(res_fn);
  
  return cf_err;
}
