#pragma once
#include <argp.h>
#include "cf.h"
#include "timemeasure.h"
#include "io.h"
#include "logger.h"

#define TEST_LOG_PRINT(tname, ...)  LOG_PRINT("[%s]: ", tname);LOG_PRINT( __VA_ARGS__)

typedef struct arguments {
  int verbose;
  int time_measure;
  uint nbits;
  char *arithfile;
  //!< i-th bit (1st bit ==> i = 0) correspond to the target i. If the bit is set,
  //!< execute that target.
  uint targets;
  int all_targets;
  char *dir;
  int fft;
  char *outfile; // write test results to a file
  cf_scheme_e scheme;
  uint d;
} test_params_t;

typedef struct {
  char *name;
  cf_errno (*function)(test_params_t *params);
  char *description;
} test_target_t;

cf_errno run_test(test_target_t *target, test_params_t *params);

extern cf_errno test_mpi_uchar_char(test_params_t *params);
extern cf_errno test_tag_mul(test_params_t *params);
extern cf_errno test_arith_circ_parse(test_params_t *params);
extern cf_errno test_eval_native(test_params_t *params);
extern cf_errno test_eval_pinocchio(test_params_t *params);
extern cf_errno test_rnd_cf(test_params_t *params);
extern cf_errno test_all_acs_one_dir(test_params_t *params);
extern cf_errno test_cf_mpi(test_params_t *params);
extern cf_errno test_all_msgevals_one_dir(test_params_t *params);
//extern cf_errno test_keygen(test_params_t *params);
