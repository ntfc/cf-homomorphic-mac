/**
 *
 * Description of test:
 *   1. Write a new (small) circuit to file
 *   2. Read the circuit from file
 *   3. Check that everything is how it is supposed to be
 *
 */
#include <stdio.h>
#include "test.h"

int is_mul_gate(const gate_t gate) {
  if(gate->type != MUL || gate->n_inputs != 2 || gate->input_ids == NULL || gate->const_val != NULL)
    return 0;
  return 1;
}
int is_add_gate(const gate_t gate) {
  if(gate->type != ADD || gate->n_inputs != 2 || gate->input_ids == NULL || gate->const_val != NULL)
    return 0;
  return 1;
}
int is_constmul_gate(const gate_t gate) {
  if(gate->type != CONSTMUL || gate->n_inputs != 1 || gate->input_ids == NULL || gate->const_val == NULL)
    return 0;
  return 1;
}

#pragma GCC diagnostic push // start of GCC ignore calls
#pragma GCC diagnostic ignored "-Wunused-parameter"
//#pragma GCC diagnostic ignored "-Wunused-variable"
cf_errno test_arith_circ_parse(test_params_t *params) {
  char fn[26] = "/tmp/test-ac-parse.XXXXXX\0";
  int in_fd;
  FILE *fp = NULL;
  arith_circ_t arith = NULL;
  //graph_t graph;
  gate_t *gates = NULL;
  cf_errno cf_err = E_NOERR;

  in_fd = mkstemp(fn);
  if(in_fd == -1) {
    cf_err = E_ERRNO_ERROR;
  }

  fp = fopen(fn, "w");
  if(fp == NULL) {
    cf_err = E_ERRNO_ERROR;
  }

  // write the arith circ to file
  fprintf(fp, "total 17\n");
  fprintf(fp, "input 0\ninput 1\ninput 2\ninput 3 # one-input\n");
  fprintf(fp, "mul in 2 <0 1> out 1 <4>\n");
  fprintf(fp, "add in 2 <0 2> out 1 <5>\n");
  fprintf(fp, "const-mul-2 in 1 <2> out 1 <6>\n");
  fprintf(fp, "const-mul-64 in 1 <3> out 1 <7>\n");
  fprintf(fp, "const-mul-a in 1 <4> out 1 <8>\n");
  fprintf(fp, "mul in 2 <5 6> out 1 <9>\n");
  fprintf(fp, "add in 2 <1 8> out 1 <10>\n");
  fprintf(fp, "add in 2 <6 7> out 1 <11>\n");
  fprintf(fp, "mul in 2 <10 11> out 1 <12>\n");
  fprintf(fp, "const-mul-a in 1 <9> out 1 <13>\n");
  fprintf(fp, "add in 2 <12 13> out 1 <14>\n");
  fprintf(fp, "const-mul-32 in 1 <12> out 1 <15>\n");
  fprintf(fp, "mul in 2 <14 15> out 1 <16>\n");
  fprintf(fp, "output 16");
  fclose(fp);

  cf_err = arith_circ_parse(&arith, fn);
  remove(fn);
  if(cf_err == E_NOERR) {
    // test that everything is ok
    if(arith_circ_n_gates(arith) != 17) {
      fprintf(stderr, "wrong number of total gates\n");
      cf_err = E_INVAL_ARITH;
    }
    else if(arith_circ_n_inputs(arith) != 4) {
     fprintf(stderr, "wrong number of inputs\n");
     cf_err = E_INVAL_ARITH;
    }
    else if(arith_circ_n_one_inputs(arith) != 1) {
      fprintf(stderr, "wrong number of one-inputs\n");
      cf_err = E_INVAL_ARITH;
    }
    else { // test gates one by one
      // INPUT GATES
      gates = arith_circ_gates(arith);
      if(gates[0]->type != INPUT || gates[1]->type != INPUT || gates[2]->type != INPUT || gates[3]->type != ONEINPUT) {
        fprintf(stderr, "wrong type for input gates\n");
        cf_err = E_INVAL_ARITH;
      }
      else if(gates[0]->n_inputs!=0 || gates[1]->n_inputs!=0 || gates[2]->n_inputs!=0 || gates[3]->n_inputs!=0) {
        fprintf(stderr, "wrong number of n_inputs for inputs (should have 0)\n");
        cf_err = E_INVAL_ARITH;
      }
      else if(gates[0]->const_val!=NULL || gates[1]->const_val!=NULL || gates[2]->const_val!= NULL || gates[3]->const_val!=NULL) {
        fprintf(stderr, "const val should be NULL for input gates\n");
        cf_err = E_INVAL_ARITH;
      }
      // OTHER GATES
      else if(! is_mul_gate(gates[4])) {
        fprintf(stderr, "gate 4 is not a valid mul gate\n");
        cf_err = E_INVAL_ARITH;
      }
      else if(! is_add_gate(gates[5])) {
        fprintf(stderr, "gate 5 is not a valid add gate\n");
        cf_err = E_INVAL_ARITH;
      }
      else if(! is_constmul_gate(gates[6])) {
        fprintf(stderr, "gate 6 is not a valid constmul gate\n");
        cf_err = E_INVAL_ARITH;
      }
      else if(! is_constmul_gate(gates[7])) {
        fprintf(stderr, "gate 7 is not a valid constmul gate\n");
        cf_err = E_INVAL_ARITH;
      }
      else if(! is_constmul_gate(gates[8])) {
        fprintf(stderr, "gate 8 is not a valid constmul gate\n");
        cf_err = E_INVAL_ARITH;
      }
      else if(! is_mul_gate(gates[9])) {
        fprintf(stderr, "gate 9 is not a valid mul gate\n");
        cf_err = E_INVAL_ARITH;
      }
      else if(! is_add_gate(gates[10])) {
        fprintf(stderr, "gate 10 is not a valid add gate\n");
        cf_err = E_INVAL_ARITH;
      }
      else if(! is_add_gate(gates[11])) {
        fprintf(stderr, "gate 11 is not a valid add gate\n");
        cf_err = E_INVAL_ARITH;
      }
      else if(! is_mul_gate(gates[12])) {
        fprintf(stderr, "gate 12 is not a valid mul gate\n");
        cf_err = E_INVAL_ARITH;
      }
      else if(! is_constmul_gate(gates[13])) {
        fprintf(stderr, "gate 13 is not a valid constmul gate\n");
        cf_err = E_INVAL_ARITH;
      }
      else if(! is_add_gate(gates[14])) {
        fprintf(stderr, "gate 14 is not a valid add gate\n");
        cf_err = E_INVAL_ARITH;
      }
      else if(! is_constmul_gate(gates[15])) {
        fprintf(stderr, "gate 15 is not a valid constmul gate\n");
        cf_err = E_INVAL_ARITH;
      }
      else if(! is_mul_gate(gates[16])) {
        fprintf(stderr, "gate 16 is not a valid mul gate\n");
        cf_err = E_INVAL_ARITH;
      }
      // TEST INPUT IDS
      else if(gates[4]->input_ids[0]!=0 || gates[4]->input_ids[1]!=1) {
        fprintf(stderr, "wrong input ids for gate 4\n");
        cf_err = E_INVAL_ARITH;
      }
      else if(gates[5]->input_ids[0]!=0 || gates[5]->input_ids[1]!=2) {
        fprintf(stderr, "wrong input ids for gate 4\n");
        cf_err = E_INVAL_ARITH;
      }
      else if(gates[6]->input_ids[0]!=2) {
        fprintf(stderr, "wrong input ids for gate 6\n");
        cf_err = E_INVAL_ARITH;
      }
      else if(gates[7]->input_ids[0]!=3) {
        fprintf(stderr, "wrong input ids for gate 7\n");
        cf_err = E_INVAL_ARITH;
      }
      else if(gates[8]->input_ids[0]!=4) {
        fprintf(stderr, "wrong input ids for gate 8\n");
        cf_err = E_INVAL_ARITH;
      }
      else if(gates[9]->input_ids[0]!=5 || gates[9]->input_ids[1]!=6) {
        fprintf(stderr, "wrong input ids for gate 9\n");
        cf_err = E_INVAL_ARITH;
      }
      else if(gates[10]->input_ids[0]!=1 || gates[10]->input_ids[1]!=8) {
        fprintf(stderr, "wrong input ids for gate 10\n");
        cf_err = E_INVAL_ARITH;
      }
      else if(gates[11]->input_ids[0]!=6 || gates[11]->input_ids[1]!=7) {
        fprintf(stderr, "wrong input ids for gate 11\n");
        cf_err = E_INVAL_ARITH;
      }
      else if(gates[12]->input_ids[0]!=10 || gates[12]->input_ids[1]!=11) {
        fprintf(stderr, "wrong input ids for gate 12\n");
        cf_err = E_INVAL_ARITH;
      }
      else if(gates[13]->input_ids[0]!=9) {
        fprintf(stderr, "wrong input ids for gate 13\n");
        cf_err = E_INVAL_ARITH;
      }
      else if(gates[14]->input_ids[0]!=12 || gates[14]->input_ids[1]!=13) {
        fprintf(stderr, "wrong input ids for gate 14\n");
        cf_err = E_INVAL_ARITH;
      }
      else if(gates[15]->input_ids[0]!=12) {
        fprintf(stderr, "wrong input ids for gate 15\n");
        cf_err = E_INVAL_ARITH;
      }
      else if(gates[16]->input_ids[0]!=14 || gates[16]->input_ids[1]!=15) {
        fprintf(stderr, "wrong input ids for gate 16\n");
        cf_err = E_INVAL_ARITH;
      }
      // TODO: outputs
      else {
        cf_err = E_NOERR;
      }
    }
    arith_circ_free(arith);
  }

  return cf_err;
}
#pragma GCC diagnostic pop // end of GCC ignore calls
