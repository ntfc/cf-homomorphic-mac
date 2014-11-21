/**
 * @file arith.h
 * @brief Definition of arithmetic circuit related functions.
 * 
 * Arithmetic circuit related functions include file-parsing functions
 * (see arithparser.c), circuit evaluation (see aritheval.c) and obviously the
 * management of the arithmetic circuit structure.
 * 
 * This is supposed to be the high level module, ie. the one that is called by
 * every other public module.
 * 
 * Here's an example on how to read an arithmetic circuit from a file to our
 * internal representation:
 * @code
 * arith_circ_t arith = NULL;
 * char fn[] = "/some/path/to/an/arithmetic/circuit";
 * cf_errno err = arith_circ_parse(&arith, fn);
 * // deal with error (see error.h)
 * ...
 * arith_circ_free(arith);
 * @endcode
 * 
 * Here is a possible use where one evaluates a set of messages over an arithmetic
 * circuit (no error checking done!):
 * @code
 * cf_message_t *msgs = NULL; // array of N_MSGS messages.
 * // ... read messages ...
 * cf_message_t eval = NULL; // result of evaluation
 * char fn[] = "/some/path/to/an/arithmetic/circuit";
 * arith_circ_t arith = NULL;
 * cf_errno err = arith_circ_parse(&arith, fn);
 * // eval msgs over the arith, with no modulo reduction.
 * err = arith_circ_eval(&eval, arith, msgs, N_MSGS, NULL);
 * ...
 * // cleanup
 * cf_message_free(eval);
 * cf_messages_free(msgs);
 * arith_circ_free(arith);
 * @endcode
 * 
 * @see aritheval.c   How arithmetic circuit evaluation is performed.
 * @see arithparser.c More information on how the arithmetic circuit parsing is
 * done.
 */ 
#pragma once
#include "graph.h"
#include "mpi-cf.h"

#ifdef __cplusplus
extern "C" {
#endif

//! Obtain the number of inputs of an arithmetic circuit.
#define arith_circ_n_inputs(a)              ((a)->n_inputs)
//! Obtain the number of one-inputs of an arithmetic circuit.
#define arith_circ_n_one_inputs(a)          ((a)->n_one_inputs)
//! Obtain the number of gates of an arithmetic circuit.
#define arith_circ_n_gates(a)               ((a)->graph->n_nodes)
//! Obtain the underlying #graph_t field of an arithmetic circuit.
#define arith_circ_graph(a)                 ((a)->graph)
//! Obtain the vector of gates of an arithmetic circuit.
#define arith_circ_gates(a)                 ((a)->gates)
//! Obtain the number of internal gates of an arithmetic circuit.
#define arith_circ_n_internal_gates(a)      ((size_t)((a)->graph->n_nodes - ((a)->n_inputs)))

/**
 * @brief Supported gate types.
 * @note The #ONEINPUT gate is used only because in pinocchio the last input 
 * gate has usually a value of 1.
 */
typedef enum {
  NONE /**< Empty gate. */, ADD /**< Addition gate. */, MUL /**< Product gate. */,
  INPUT /**< Input gate. */, ONEINPUT /**< One-input gate. */,
  CONSTMUL /**< Constant multiplication gate. */
} type_e;

/**
 * @brief Definition a gate structure.
 * 
 * A gate is simply composed by the following:
 *  * Gate type;
 *  * Number of gate inputs;
 *  * Gate's Input ID's. In the case of arithmetic circuits, no more than 2 inputs
 * are allowed.
 *  * Constant value. Only applicable when gate_t#type is #CONSTMUL.
 */
typedef struct gate_s {
  type_e type;
  size_t n_inputs;
  edge_id_t *input_ids;
  cf_mpi_t const_val;
} *gate_t;

/**
 * @brief Definition of an arithmetic circuit.
 * 
 * An arithmetic circuit structure is composed by the following fields:
 *  * number of input gates (arith_circ_t#n_inputs);
 *  * number of one-input gates (arith_circ_t#n_one_inputs);
 *  * a (adjacency list graph) #graph_t containing all the edges (arith_circ_t#graph);
 *  * a vector of #gate_t's that describes every gate of the circuit (arith_circ_t#gates);
 *  * the file that describes the arithmetic circuit (arith_circ_t#fn);
 *  * a vector of gate ID's calculated using topological sort (arith_circ_t#t_sort).
 */
struct arith_circ_s {
  size_t n_inputs; //!< Number of input gates. Same value as in graph_t#n_inputs.
  size_t n_one_inputs; //!< Number of one-input gates.
  graph_t graph; //!< Graph containg all the edges.
  gate_t *gates; //!< Vector containing every gate of the circuit.
  char *fn; //!< The file associated that contains the arithmetic circuit.
  gate_id_t *t_sort; //!< Topological sort for the arithmetic circuit.
};
//! An arithmetic circuit is a pointer to arith_circ_s;
typedef struct arith_circ_s *arith_circ_t;

/**
 * @brief Initialization of an arithmetic circuit.
 * 
 * All number fields are initialized as 0. The arith_circ_t#graph, arith_circ_t#gates
 * and arith_circ_t#fn fields are set to \c NULL.
 */
void         arith_circ_init(arith_circ_t*);

/**
 * @brief Initialization of the arith_circ_t#graph field.
 * 
 * Given a number of gates, a newly graph is allocated and properly initialized
 * to zero values.
 */
void         arith_circ_init_gates(arith_circ_t, size_t);

/**
 * @brief Releases the memory used by a previously allocated arithmetic circuit.
 * 
 * All allocated memory within an arithmetic circuit is free'd. This also includes
 * all memory allocated for the arith_circ_t#graph field.
 */
void         arith_circ_free(arith_circ_t);

//! @note Debug purposes only.
void         arith_circ_print(const arith_circ_t);

/**
 * @brief Adds the gate to the arithmetic circuit graph.
 */
cf_errno     arith_circ_add_gate(arith_circ_t, gate_id_t, gate_t);

/**
 * @brief Performs the topological sort over the arith_circ_t#graph field.
 */
cf_errno     arith_circ_topology_sort(arith_circ_t);

/**
 * @brief Performs the arithmetic circuit evaluation using pinocchio.
 * 
 * @see pinocchio.h Necessary to define the pinocchio path on this file.
 */
cf_errno   arith_circ_eval_pinocchio(cf_mpi_t *, const arith_circ_t, cf_mpi_t *, size_t, const cf_mpi_t);

/**
 * @brief Native version of the arithmetic circuit evaluation.
 */
cf_errno     arith_circ_eval(cf_mpi_t *, const arith_circ_t, cf_mpi_t *, size_t, const cf_mpi_t);

/**
 * @brief Converts a given arithmetic circuit represented in a file, and converts
 * to our internal representation.
 */
cf_errno      arith_circ_parse(arith_circ_t *, const char *);

// I/O reading/writing functions
cf_mpi_t*     arith_circ_parse_io(const char *, size_t *);
void          arith_circ_write_io(const char *, const cf_mpi_t *, size_t);

gate_t        gate_create(type_e, gate_id_t *, size_t, cf_mpi_t);
void          gate_copy(const gate_t, gate_t *);
void          gate_free(gate_t);

#ifdef __cplusplus
}
#endif
