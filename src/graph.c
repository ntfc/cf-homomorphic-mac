#include "graph.h"

graph_al_t graph_al_create(size_t nodes) {
  size_t i;
  graph_al_t g = (graph_al_t)calloc(1, sizeof(struct graph_al_s));
  assert(g != NULL);
  g->n_nodes = nodes;
  g->adj_lists = (edge_t*)calloc(nodes, sizeof(edge_t));
  g->n_inputs = 0;
  for(i = 0; i < g->n_nodes; i++) {
    g->adj_lists[i] = NULL;
  }
  return g;
}

void graph_al_free(graph_al_t g) {
  size_t i = 0;
  s_edge_t *next = NULL, *aux = NULL;
  if(g != NULL) {
    for(i = 0; i < g->n_nodes; i++) {
      // free the edges
      if(g->adj_lists != NULL && g->adj_lists[i] != NULL) {
        // free the linked list
        next = g->adj_lists[i]->next;
        while(next != NULL) {
          aux = next;
          next = next->next;
          free(aux);
          aux = NULL;
        }
        free(g->adj_lists[i]);
        g->adj_lists[i] = NULL;
      }
      /*// free the gates
      if(g->gate_lists != NULL && g->gate_lists[i] != NULL) {
        gate_free(g->gate_lists[i]);
      }*/
    }
    if(g->adj_lists != NULL) {
      free(g->adj_lists);
      g->adj_lists = NULL;
    }
    /*if(g->gate_lists != NULL) {
      free(g->gate_lists);
      g->gate_lists = NULL;
    }*/
    free(g);
    g = NULL;
  }
}

int graph_al_add_edge(graph_al_t g, edge_id_t source, edge_id_t dest) {
  s_edge_t *new = NULL;
  
  if (g && (source < g->n_nodes) && (dest < g->n_nodes)) {
    new = (s_edge_t*)calloc(1, sizeof(s_edge_t));
    assert(new != NULL);
    new->dest = dest;
    new->next = g->adj_lists[source];
    g->adj_lists[source] = new;
    return 0;
  }
  else
    return(-1);
}

// just return the number of output gates of gate_id
uint16_t graph_al_get_gate_n_out_deps(const graph_al_t g, edge_id_t gate_id) {
  s_edge_t *aux = NULL;
  size_t n_deps = 0;
  if(g && (gate_id < g->n_nodes) && g->adj_lists[gate_id]) {
    aux = g->adj_lists[gate_id];
    while(aux) {
      n_deps++;
      assert(n_deps < UINT16_MAX);
      aux = aux->next;
    }
  }
  else {
    n_deps = 0;
  }
  return n_deps;
}

// just return the edge_id_t list with the dependecies IDs
void graph_al_get_gate_out_deps(const graph_al_t g, edge_id_t gate_id, edge_id_t **deps, size_t *n_deps) {
  s_edge_t *aux = NULL;
  edge_id_t *aux_ptr = NULL;
  size_t alloced = 0;
  if(*deps != NULL) {
    free(*deps);
    *deps = NULL;
  }
  if(g && (gate_id < g->n_nodes) && g->adj_lists[gate_id]) {
    aux = g->adj_lists[gate_id];
    while(aux) {
      (*n_deps)++;
      if(alloced < *n_deps) {
        // it is not very efficient to only alloc one element at a time
        // instead, alloc always twice as much space needed 
        aux_ptr = realloc(*deps, ((*n_deps)*2) * sizeof(edge_id_t));
        assert(aux_ptr != NULL);
        if(aux_ptr != NULL) {
          *deps = aux_ptr;
          alloced = (*n_deps) * 2;
        }
      }
      (*deps)[(*n_deps) - 1] = aux->dest;
      aux = aux->next;
    }
  }
  else {
    *deps = NULL;
    *n_deps = 0;
  }
}
