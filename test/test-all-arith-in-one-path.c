/**
 * @brief Test all arithmetic circuits in a given directory.
 * 
 * Given a directory, this test checks that the CF protocol is well executed on
 * all of them. Even if the CF protocol only fails on verification, the test is
 * not interrupted (i.e., the remaining ACs are tested).
 * 
 * This test is useful to test the execution times of the protocol, but only if
 * the \c --time option is passed. In that case, a timing summary is presented
 * in the end with the worst, best and average timings.
 */

#include <stdio.h>
#include <dirent.h>
#include "cf-random.h"
#include "test.h"
#include "operations.h"
#include "keys.h"

#define N_MEASURES 4 // 0=msgeval, 1=auth, 2=eval, 3=vrfy

static ulong cpu_worst[N_MEASURES]; // 0=msgeval, 1=auth, 2=eval, 3=vrfy
static ulong wall_worst[N_MEASURES]; // 0=msgeval, 1=auth, 2=eval, 3=vrfy
static ulong cpu_best[N_MEASURES]; // 0=msgeval, 1=auth, 2=eval, 3=vrfy
static ulong wall_best[N_MEASURES]; // 0=msgeval, 1=auth, 2=eval, 3=vrfy
static ulong cpu_total[N_MEASURES]; // 0=msgeval, 1=auth, 2=eval, 3=vrfy
static ulong wall_total[N_MEASURES]; // 0=msgeval, 1=auth, 2=eval, 3=vrfy
static uint count = 0;

// test name. for logging purposes
static char TNAME[] = "allACdir";

static FILE *outfile = NULL;

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

static void _set_global_timings(cf_time_t msgeval, cf_time_t auth, cf_time_t eval,
                                cf_time_t vrfy) {
  if(count > 1) { // not the first AC
    if(CPU_TOTAL(msgeval) < cpu_best[0]) cpu_best[0] = CPU_TOTAL(msgeval);
    if(CPU_TOTAL(auth) < cpu_best[1]) cpu_best[1] = CPU_TOTAL(auth);
    if(CPU_TOTAL(eval) < cpu_best[2]) cpu_best[2] = CPU_TOTAL(eval);
    if(CPU_TOTAL(vrfy) < cpu_best[3]) cpu_best[3] = CPU_TOTAL(vrfy);
    
    if(CPU_TOTAL(msgeval) > cpu_worst[0]) cpu_worst[0] = CPU_TOTAL(msgeval);
    if(CPU_TOTAL(auth) > cpu_worst[1]) cpu_worst[1] = CPU_TOTAL(auth);
    if(CPU_TOTAL(eval) > cpu_worst[2]) cpu_worst[2] = CPU_TOTAL(eval);
    if(CPU_TOTAL(vrfy) > cpu_worst[3]) cpu_worst[3] = CPU_TOTAL(vrfy);
    
    if(WALL_TOTAL(msgeval) < wall_best[0]) wall_best[0] = WALL_TOTAL(msgeval);
    if(WALL_TOTAL(auth) < wall_best[1]) wall_best[1] = WALL_TOTAL(auth);
    if(WALL_TOTAL(eval) < wall_best[2]) wall_best[2] = WALL_TOTAL(eval);
    if(WALL_TOTAL(vrfy) < wall_best[3]) wall_best[3] = WALL_TOTAL(vrfy);
    
    if(WALL_TOTAL(msgeval) > wall_worst[0]) wall_worst[0] = WALL_TOTAL(msgeval);
    if(WALL_TOTAL(auth) > wall_worst[1]) wall_worst[1] = WALL_TOTAL(auth);
    if(WALL_TOTAL(eval) > wall_worst[2]) wall_worst[2] = WALL_TOTAL(eval);
    if(WALL_TOTAL(vrfy) > wall_worst[3]) wall_worst[3] = WALL_TOTAL(vrfy);
    
    cpu_total[0] += CPU_TOTAL(msgeval);   wall_total[0] += WALL_TOTAL(msgeval);
    cpu_total[1] += CPU_TOTAL(auth);      wall_total[1] += WALL_TOTAL(auth);
    cpu_total[2] += CPU_TOTAL(eval);      wall_total[2] += WALL_TOTAL(eval);
    cpu_total[3] += CPU_TOTAL(vrfy);      wall_total[3] += WALL_TOTAL(vrfy);
  }
  else { // the first AC
    cpu_best[0] = CPU_TOTAL(msgeval);   cpu_worst[0] = CPU_TOTAL(msgeval);
    cpu_best[1] = CPU_TOTAL(auth);      cpu_worst[1] = CPU_TOTAL(auth);
    cpu_best[2] = CPU_TOTAL(eval);      cpu_worst[2] = CPU_TOTAL(eval);
    cpu_best[3] = CPU_TOTAL(vrfy);      cpu_worst[3] = CPU_TOTAL(vrfy);
    
    wall_best[0] = WALL_TOTAL(msgeval);   wall_worst[0] = WALL_TOTAL(msgeval);
    wall_best[1] = WALL_TOTAL(auth);      wall_worst[1] = WALL_TOTAL(auth);
    wall_best[2] = WALL_TOTAL(eval);      wall_worst[2] = WALL_TOTAL(eval);
    wall_best[3] = WALL_TOTAL(vrfy);      wall_worst[3] = WALL_TOTAL(vrfy);
    
    cpu_total[0] = CPU_TOTAL(msgeval);   wall_total[0] = WALL_TOTAL(msgeval);
    cpu_total[1] = CPU_TOTAL(auth);      wall_total[1] = WALL_TOTAL(auth);
    cpu_total[2] = CPU_TOTAL(eval);      wall_total[2] = WALL_TOTAL(eval);
    cpu_total[3] = CPU_TOTAL(vrfy);      wall_total[3] = WALL_TOTAL(vrfy);
  }
}

static cf_errno _execute_cf(const char *fn, test_params_t *params,
                            cf_randstate_t prng) {
  cf_errno cf_err = E_NOERR;
  FILE *fp = NULL;
  cf_keys_t keys = NULL;
  arith_circ_t arith = NULL;
  cf_message_t *msgs = NULL, msg_eval = NULL;
  cf_label_t *lbls = NULL;
  cf_tag_t *tags = NULL, mac = NULL, local_tag = NULL;
  size_t n = 0, i = 0, n_mul, n_add, n_const;
  cf_program_t prgm = NULL;
  prf_hd_t prf = NULL;

  fp = fopen(fn, "r");
  if(fp == NULL)
    return E_ERRNO_ERROR;
  // parse arith
  START_MEASURE(parse);
  cf_err = arith_circ_parse(&arith, fn);
  END_MEASURE(parse);
  if(cf_err != E_NOERR) goto cleanup;
  n = arith_circ_n_inputs(arith);
  _arith_circ_n_mul_add_const_gates(arith, &n_mul, &n_add, &n_const);
  fprintf(stderr, "Executing %d '%s': (%zu inputs, %zu gates, FFT %s, %u bits)\n", count+1, fn, n, arith_circ_n_gates(arith),
      params->fft == 1 ? "ON" : "OFF", params->nbits);
  fprintf(stderr, "  - mul gates: %zu, add gates: %zu, const-mul gates: %zu\n",
                   n_mul, n_add, n_const);

  // generate keys
  START_MEASURE(keygen);
  cf_err = cf_key_gen(&keys, params->nbits, params->d, params->scheme, prng);
  END_MEASURE(keygen);
  if(cf_err != E_NOERR)
    goto cleanup;
  TEST_LOG_PRINT(TNAME, "Keys generated\n");
  PRINT_MEASURE("  - keygen", keygen);
  // prf handler
  cf_err = prf_create_handler(&prf, keys->sk->k, params->nbits / 8);
  if(cf_err != E_NOERR)
    goto cleanup;
  // generate messages and labels
  cf_messages_random(&msgs, n, keys->ek->p, prng);
  cf_labels_random(&lbls, n, params->nbits, prng);
  
  // msgeval
  START_MEASURE(msgeval);
  cf_err = arith_circ_eval(&msg_eval, arith, msgs, n, keys->ek->p);
  END_MEASURE(msgeval);
  PRINT_MEASURE("  - msgeval", msgeval);
  if(cf_err != E_NOERR) goto cleanup;
  // auth
  cf_tags_new(&tags, n);
  START_MEASURE(auth);
  for(i = 0; i < n; i++) {
    cf_err = cf_auth(&(tags[i]), keys->sk, lbls[i], msgs[i], prf);
    if(cf_err != E_NOERR) goto cleanup;
  }
  END_MEASURE(auth);
  PRINT_MEASURE("  - auth", auth);
  // eval
  // we can already do some cleaup
  cf_messages_free(msgs, n); msgs = NULL;
  START_MEASURE(eval);
  cf_err = cf_eval(&mac, keys->ek, arith, tags, n, &local_tag, params->scheme);
  END_MEASURE(eval);
  PRINT_MEASURE("  - eval", eval);
  if(cf_err != E_NOERR) goto cleanup;
  // vrfy
  // we can already do some cleaup
  cf_tags_free(tags, n); tags = NULL;
  cf_program_init(&prgm, lbls, n, arith);
  START_MEASURE(vrfy);
  cf_err = cf_vrfy(keys->sk, msg_eval, prgm, mac, prf);
  END_MEASURE(vrfy);
  PRINT_MEASURE("  - vrfy", vrfy);
  
  if(cf_err == E_VRFY) {
    LOG_PRINT("vrfy ok\n");
    cf_err = E_NOERR;
  }

  count++;
  if(params->time_measure)
    _set_global_timings(msgeval, auth, eval, vrfy);

  if(outfile != NULL) {
    fprintf(outfile, "%zu\t%zu\t%zu\t%zu\t%zu\t", arith_circ_n_gates(arith), n, n_mul, n_add, n_const);
    if(params->time_measure) {
      FPRINT_WALL_TIME(outfile, parse);
      fprintf(outfile, "\t");
      FPRINT_WALL_TIME(outfile, msgeval);
      fprintf(outfile, "\t");
      FPRINT_WALL_TIME(outfile, keygen);
      fprintf(outfile, "\t");
      FPRINT_WALL_TIME(outfile, auth);
      fprintf(outfile, "\t");
      FPRINT_WALL_TIME(outfile, eval);
      fprintf(outfile, "\t");
      FPRINT_WALL_TIME(outfile, vrfy);
      fprintf(outfile, "\t");
    }
    fprintf(outfile, "\n");
  }
cleanup:
  fclose(fp);
  arith_circ_free(arith);
  cf_messages_free(msgs, n);
  cf_labels_free(lbls, n);
  cf_message_free(msg_eval);
  cf_tags_free(tags, n);
  cf_tag_free(local_tag);
  cf_tag_free(mac);
  cf_program_free(prgm);
  cf_keys_free(keys);
  prf_close_handler(prf);
  

  return cf_err;
}

cf_errno test_all_acs_one_dir(test_params_t *params) {
  cf_errno cf_err = E_NOERR;
  DIR *d = NULL;
  struct dirent *dir = NULL;
  int i;
  cf_randstate_t prng;
  
  if(params->dir == NULL) {
    return E_NO_DIR; // no directory
  }
  d = opendir(params->dir);
  if(d == NULL) {
    return E_ERRNO_ERROR;
  }
  // open outfile before changing dir
  if(params->outfile != NULL) {
    outfile = fopen(params->outfile,"w");
    assert(outfile != NULL);
  }

  // chdir
  if(chdir(params->dir) != 0) {
    cf_err = E_ERRNO_ERROR;
    goto cleanup;
  }
  // prng init
  if( (cf_err = cf_mpi_library_init(&prng)) != E_NOERR)
    goto cleanup;
  // turn off FFT if necessary
  use_fft = params->fft;
  count = 0;
  if(outfile != NULL) {
    fprintf(outfile, "# test all circuits in %s with %u bits\n", params->dir, params->nbits);
    if(params->time_measure) {
      fprintf(outfile, "gates\tinputs\tmuls\tadd\tconsts\tparse\tmsgeval\tkeygen\tauth\teval\tvrfy\t\n");
    }
  }
  while( (dir = readdir(d)) != NULL) {
    if(_is_arith_filename(dir->d_name)) { // .arith extension present
      // parse and execute CF
      TEST_LOG_PRINT(TNAME, "trying to execute CF-%d on '%s'", params->scheme, dir->d_name);
      cf_err = _execute_cf(dir->d_name, params, prng);
      if(cf_err != E_NOERR)
        fprintf(stderr, "AC failed to vrfy: %s\n", str_cferror(cf_err));
      fflush(stderr);
      if(cf_err != E_NOERR)
        goto cleanup;
    }
  }
  
  if(outfile != NULL) {
    fprintf(outfile, "# total: %u,  FFT = %s\n", count, (use_fft==1?"ON":"OFF"));
  }
cleanup:
  closedir(d);
  cf_mpi_library_free(prng);
  if(outfile != NULL) {
    if(cf_err != E_NOERR) {
      fprintf(outfile, "# test failed with code %d\n", cf_err);
    }
    fclose(outfile);
  }

  if(count > 0 && params->time_measure) {
    // print worst, best and avg timings
    fprintf(stderr, "\nTimings summary:\n");
    fprintf(stderr, "  - total nr of circuits: %u, FFTmul %s, %u bits\n", count, (use_fft == 1 ? "ON" : "OFF"), params->nbits);
    for(i = 0; i < N_MEASURES; i++) {
      switch(i) {
        case 0: fprintf(stderr, "  - msgeval");break;
        case 1: fprintf(stderr, "  - auth");break;
        case 2: fprintf(stderr, "  - eval");break;
        case 3: fprintf(stderr, "  - vrfy");break;
      }
      fprintf(stderr, " average: wall=%.2fms;cpu=%.2fms", (double)wall_total[i]/(double)count, (double)cpu_total[i]/(double)count);
      fprintf(stderr, " best: wall=%lums;cpu=%lums", wall_best[i], cpu_best[i]);
      fprintf(stderr, " worst: wall=%lums;cpu=%lums", wall_worst[i], cpu_worst[i]);
      fprintf(stderr, "\n");
      fflush(stderr);
    }
  }

  
  return cf_err;
}
