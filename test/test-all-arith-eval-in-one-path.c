/**
 * @brief Performs a circuit evaluation with random messages on all arithmetic
 * circuits present on a given directory.
 */

#include <stdio.h>
#include <dirent.h>
#include "operations.h"
#include "cf-random.h"
#include "test.h"

static ulong cpu_worst;
static ulong wall_worst; 
static ulong cpu_best; 
static ulong wall_best; 
static ulong cpu_total;
static ulong wall_total;
static uint count = 0;

// test name. for logging purposes
static char TNAME[] = "allMsgEval";

static void _arith_circ_n_mul_add_const_gates(const arith_circ_t arith,
                                size_t *n_mul, size_t *n_add, size_t *n_const) {
  size_t i;
  *n_mul = 0;
  *n_add = 0;
  *n_const = 0;
  for(i = 0; i < arith_circ_n_gates(arith); i++) {
    if(arith_circ_gates(arith)[i]->type == MUL)
      (*n_mul)++;
    if(arith_circ_gates(arith)[i]->type == ADD)
      (*n_add)++;
    if(arith_circ_gates(arith)[i]->type == CONSTMUL)
      (*n_const)++;
  }
}

static inline int _is_arith_filename(const char *name) {
  size_t dir_len = strlen(name);
  if(dir_len > 6) {
    if(strncmp(name + (dir_len - 6), ".arith", 6) == 0)
      return 1;
  }
  return 0;
}

static void _set_global_timings(cf_time_t msgeval) {
  if(count > 1) { // not the first AC
    if(CPU_TOTAL(msgeval) < cpu_best) cpu_best = CPU_TOTAL(msgeval);
    if(CPU_TOTAL(msgeval) > cpu_worst) cpu_worst = CPU_TOTAL(msgeval);
    if(WALL_TOTAL(msgeval) < wall_best) wall_best = WALL_TOTAL(msgeval);
    if(WALL_TOTAL(msgeval) > wall_worst) wall_worst = WALL_TOTAL(msgeval);

    cpu_total += CPU_TOTAL(msgeval);   wall_total += WALL_TOTAL(msgeval);
  }
  else { // the first AC
    cpu_best = CPU_TOTAL(msgeval);   cpu_worst = CPU_TOTAL(msgeval);
    wall_best = WALL_TOTAL(msgeval);   wall_worst = WALL_TOTAL(msgeval);
    cpu_total = CPU_TOTAL(msgeval);   wall_total = WALL_TOTAL(msgeval);
  }
}

static cf_errno _execute_msgeval(const char *fn, test_params_t *params, const
    cf_mpi_t mod, cf_randstate_t prng) {
  cf_errno cf_err = E_NOERR;
  FILE *fp = NULL;
  arith_circ_t arith = NULL;
  cf_message_t *msgs = NULL, msg_eval = NULL;
  size_t n = 0, n_mul, n_add, n_const;

  fp = fopen(fn, "r");
  if(fp == NULL)
    return E_ERRNO_ERROR;
  // parse arith

  START_MEASURE(parse);
  cf_err = arith_circ_parse(&arith, fn);
  END_MEASURE(parse);
  PRINT_MEASURE("  - parse", parse);

  if(cf_err != E_NOERR) goto cleanup;
  n = arith_circ_n_inputs(arith);
  _arith_circ_n_mul_add_const_gates(arith, &n_mul, &n_add, &n_const);
  fprintf(stderr, "Executing %d '%s': (%zu inputs, %zu gates, FFT %s, %u bits)\n", count+1, fn, n, arith_circ_n_gates(arith),
      params->fft == 1 ? "ON" : "OFF", params->nbits);
  fprintf(stderr, "  - mul gates: %zu, add gates: %zu, const-mul gates: %zu\n",
                   n_mul, n_add, n_const);
  // generate messages
  cf_messages_random(&msgs, n, mod, prng);
  
  // msgeval
  START_MEASURE(msgeval);
  cf_err = arith_circ_eval(&msg_eval, arith, msgs, n, mod);
  END_MEASURE(msgeval);
  PRINT_MEASURE("  - parse", parse);
  PRINT_MEASURE("  - msgeval", msgeval);
  if(cf_err != E_NOERR) goto cleanup;

  count++;
  if(params->time_measure)
    _set_global_timings(msgeval);

cleanup:
  fclose(fp);
  arith_circ_free(arith);
  cf_messages_free(msgs, n);
  cf_message_free(msg_eval);

  return cf_err;
}

cf_errno test_all_msgevals_one_dir(test_params_t *params) {
  cf_errno cf_err = E_NOERR;
  DIR *d = NULL;
  struct dirent *dir = NULL;
  cf_mpi_t mod = NULL;
  cf_randstate_t prng;

  if(params->dir == NULL) {
    return E_NO_DIR; // no directory
  }
  d = opendir(params->dir);
  if(d == NULL) {
    return E_ERRNO_ERROR;
  }
  // chdir
  if(chdir(params->dir) != 0) {
    cf_err = E_ERRNO_ERROR;
    goto cleanup;
  }
  // prng init
  if( (cf_err = cf_mpi_library_init(&prng)) != E_NOERR)
    goto cleanup;
  // generate modulo
  if( (cf_err = cf_mpi_random_prime(&mod, params->nbits, prng)) != E_NOERR)
    goto cleanup;
  
  TEST_LOG_PRINT(TNAME, "Keys generated\n");
  // turn off FFT if necessary
  use_fft = params->fft;
  count = 0;

  while( (dir = readdir(d)) != NULL) {
    if(_is_arith_filename(dir->d_name)) { // .arith extension present
      // parse and execute CF
      TEST_LOG_PRINT(TNAME, "trying to execute CF on '%s'", dir->d_name);
      cf_err = _execute_msgeval(dir->d_name, params, mod, prng);
      if(cf_err != E_NOERR)
        fprintf(stderr, "AC failed to vrfy: %s\n", str_cferror(cf_err));
      fflush(stderr);
      if(cf_err != E_NOERR)
        goto cleanup;
    }
  }
  
cleanup:
  closedir(d);
  cf_mpi_free(mod);
  cf_mpi_library_free(prng);

  if(count > 0 && params->time_measure) {
    // print worst, best and avg timings
    fprintf(stderr, "\nTimings summary:\n");
    fprintf(stderr, "  - total nr of circuits: %u, FFTmul %s, %u bits\n", count, (use_fft == 1 ? "ON" : "OFF"), params->nbits);

    fprintf(stderr, "  - msgeval");
    fprintf(stderr, " average: wall=%.2fms;cpu=%.2fms", (double)wall_total/(double)count, (double)cpu_total/(double)count);
    fprintf(stderr, " best: wall=%lums;cpu=%lums", wall_best, cpu_best);
    fprintf(stderr, " worst: wall=%lums;cpu=%lums", wall_worst, cpu_worst);
    fprintf(stderr, "\n");
    fflush(stderr);
  }

  return cf_err;
}
