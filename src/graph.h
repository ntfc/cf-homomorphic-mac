/**
 * Functions of this module are not supposed to be called directly by upper
 * levels modules!
 */

#pragma once

#include "error.h"

#ifdef __cplusplus
extern "C" {
#endif

#define graph_create                  graph_al_create
#define graph_al_add_edge             graph_add_edge
#define graph_al_free                 graph_free
#define graph_al_get_gate_n_out_deps  graph_get_gate_n_out_deps
#define graph_al_get_gate_out_deps    graph_get_gate_out_deps

typedef struct edge_s {
  edge_id_t dest;
  struct edge_s *next;
} s_edge_t, *edge_t;

typedef struct graph_al_s {
  size_t n_nodes; // number of total gates
  edge_t *adj_lists;
  size_t n_inputs;
} *graph_al_t;

typedef graph_al_t graph_t;

graph_al_t  graph_al_create(size_t n_nodes);
int         graph_al_add_edge(graph_al_t g, edge_id_t source, edge_id_t dest);

void        graph_al_free(graph_al_t g);

uint16_t    graph_al_get_gate_n_out_deps(const graph_al_t g, gate_id_t gate_id);
void        graph_al_get_gate_out_deps(const graph_al_t g, gate_id_t gate_id, gate_id_t **deps, size_t *n_deps);

#ifdef __cplusplus
}
#endif
