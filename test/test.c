#define _GNU_SOURCE // because of asprintf
#include <stdio.h>
#include "logger.h"
#include "test.h"

static struct argp_option opts_test[] = {
  //name      key  arg     flags   doc                                 group
  { "lambda", 'n', "NUM",  0,    "Security parameter of NUM bits (default = 128)", 0 },
  { "time",    301, 0,     0,    "Show execution times of the protocol", 0 },
  { "arith",   302,"AC",   0,    "Arithmetic Circuit file to test", 0},
  { "verbose", 'v', 0,     0,    "Verbose", 0},
  { "dir",     304,"DIR",  0,    "Directory to use",  0},
  { "no-fft",  305,0,      0,    "Disable FFT (FFT is ON by default)", 0},
  { "out",    'o', "FILE", 0,    "Where to write test results", 0}, 
  { "degree", 'D', "NUM",  0,    "Maximum degree allowed for CF-2"               , 0 },
  { "scheme", 'c', "NUM",  0,    "Use 1 for CF-1 or 2 for CF-2", 0 },
  { 0 }
};
static void full_help(struct argp_state *state);
static char *help_filter(int key, const char *text, void *input);
static error_t parse_test(int key, char *arg, struct argp_state *state);
static struct argp test_argp = { opts_test, parse_test, "TARGET ..", NULL , NULL, help_filter, NULL };

test_target_t targets[] = {
  { "mpi-convs",     test_mpi_uchar_char,   "Test MPI conversions"                    },
  { "tagmul",        test_tag_mul       ,   "Tag multiplication with and without FFT" },
  { "acparse",       test_arith_circ_parse, "Test small AC parsing"                   },
  { "pinocchio-eval",test_eval_pinocchio,   "AC eval using pinocchio"                 },
  { "native-eval",   test_eval_native,      "AC eval using native code"               },
  { "cf",            test_rnd_cf,           "Test CF protocol with a AC"              },
  { "allACdir",      test_all_acs_one_dir,  "(CAN BE SLOW) test all ACs in a given dir" },
  { "cf-mpi",        test_cf_mpi,           "Test cf-mpi datatype",                   },
  { "allMsgEvals",   test_all_msgevals_one_dir, "Execute msgeval on all circuits" },
//  { "keygen",        test_keygen,           "Test the KeyGen parameters"              },
};

const unsigned num_targets = sizeof(targets) / sizeof(targets[0]);

int main(int argc, char **argv) {
  cf_errno cf_err = E_NOERR;
  
  struct arguments params = {
    .verbose = 0, .time_measure = 0, .nbits = 128, .arithfile = NULL,
    .targets = 0, .all_targets = 0, .dir = NULL, .fft = 1, .outfile = NULL,
    .scheme = CF_1, .d = 0,
    };
  VERBOSE_OFF;
  cf_err = argp_parse(&test_argp, argc, argv, 0, 0, &params);
  if(cf_err != E_NOERR) {
    (void) str_cferror(cf_err);
  }
  return cf_err;
}

static error_t parse_test(int key, char *arg, struct argp_state *state) {
  cf_errno cf_err = E_NOERR;
  test_params_t *params = state->input;
  int target_found = 0, all_success = 1;
  uint i;
  switch(key) {
    case ARGP_KEY_NO_ARGS:
      full_help(state);
      break;
    case 'n':
      if(arg)
        cf_err = str_to_uint(&(params->nbits), arg, 10);
        if(cf_err != E_NOERR)
          cf_perror("test_parse", cf_err);
      break;
    case 301: // time measure
      params->time_measure = 1; TIME_MEASURE_ON;
      break;
    case 302: // AC file
      params->arithfile = arg;
      break;
    case 'o': // out file
      params->outfile = arg;
      break;
    case 'v': // verbose
      params->verbose = 1; VERBOSE_ON;
      break;
    case 304: // directory
      params->dir = arg;
      break;
    case 305: // no-fft
      params->fft = 0;
      break;
    case 'c': // scheme
      if(strlen(arg) == 1 && (arg[0] == '1' || arg[0] == '2')) {
        switch(arg[0]) {
          case '1': params->scheme = CF_1; break;
          case '2': params->scheme = CF_2; break;
        }
      }
      else {
        argp_error(state, "%s (expected scheme to be 1 or 2)", str_cferror(E_INVAL_NUM));
      }
      break;
    case 'D': // degree
      if( (cf_err = str_to_uint(&(params->d), arg, 10)) != E_NOERR) {
        argp_error(state, "%s", str_cferror(cf_err));
      }
      break;
    case ARGP_KEY_ARG: // target to execute
      for(i = 0; i < num_targets; i++) {
        if(strcmp(targets[i].name, arg) == 0) {
          // target found
          // set bit i+1
          params->targets ^= (1 << i); // set bit i
          target_found = 1;
        }
      }
      if(strncmp("all", arg, strlen(arg)) == 0) {
        params->all_targets = 1;
        target_found = 1;
      }
      if(!target_found) {
        fprintf(stderr, "Invalid target '%s', skipping it.\n", arg);
        argp_state_help(state, stderr, ARGP_HELP_POST_DOC);
      }
      break;
    case ARGP_KEY_FINI: // finished parsing
      if(params->targets == 0 && params->all_targets == 0) {
        argp_failure(state, 1, 0, "No valid targets found.");
      }
      // execute targets
      for(i = 0; i < num_targets; i++) {
        if(params->all_targets || ((params->targets >> i) & 1)) {
          cf_err = run_test(&(targets[i]), params);
          all_success = all_success && (cf_err == E_NOERR);
        }
      }
      fprintf(stderr, "\nSummary of tests: %s\n", all_success == 1 ? "ALL PASS" : "SOME FAIL");
      break;
 }
 return cf_err; 
}

static inline void full_help(struct argp_state *state) {
  // show usage, long help and list of targets. in the end, exit with error
  argp_state_help(state, stderr, ARGP_HELP_SHORT_USAGE|ARGP_HELP_LONG|ARGP_HELP_POST_DOC|ARGP_HELP_EXIT_ERR);
}

#pragma GCC diagnostic push // start of GCC ignore calls
#pragma GCC diagnostic ignored "-Wunused-parameter"
//#pragma GCC diagnostic ignored "-Wunused-variable"
//! @brief When help is being shown, in the end the list of targets is also shown.
static char *help_filter(int key, const char *text, void *input) {
  char *new_text = NULL, *aux = NULL;
  uint i;
  int ret = 0;
  if(key == ARGP_KEY_HELP_POST_DOC) {
    ret = asprintf(&new_text, "Valid test targets:\n  all: executes all targets\n");
    if(ret == -1) return NULL;
    for(i = 0; i < num_targets; i++) {
      if(new_text != NULL) {
        ret = asprintf(&aux, "%s", new_text);
        free(new_text);
        new_text = NULL;
        if(ret == -1) return NULL;
      }
      if(i == num_targets-1) {
        ret = asprintf(&new_text, "%s  %s: %s\n\n", aux, targets[i].name, targets[i].description);
        if(ret == -1) return NULL;
      }
      else {
        ret = asprintf(&new_text, "%s  %s: %s\n", aux, targets[i].name, targets[i].description);
        if(ret == -1) return NULL;
      }
      free(aux); aux = NULL;
    }
    
    return new_text;
  }
  return (char*) text;
}
#pragma GCC diagnostic pop // end of GCC ignore calls

cf_errno run_test(test_target_t *target, test_params_t *params) {
  fprintf(stderr, "starting %s()...\n", target->name);
  fflush(stderr);
  
  // activate time measure if required
  if(params->time_measure)
    TIME_MEASURE_ON;
  cf_errno cf_err = target->function(params);
  TIME_MEASURE_OFF;
  fflush(stderr);
  fprintf(stderr, "end of %s(): %s\n", target->name, cf_err == E_NOERR ? "OK" : str_cferror(cf_err));

  return cf_err;
}

