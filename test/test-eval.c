#include "test.h"
#include "cf-random.h"
#include "keys.h"

cf_errno test_eval_pinocchio(test_params_t *params) {
  cf_errno cf_err = E_NOERR;
  arith_circ_t arith = NULL;
  cf_randstate_t prng = NULL;
  size_t n_inputs = 0;
  cf_message_t *msgs = NULL;
  cf_message_t msgeval = NULL, mod = NULL;
  
  cf_err = cf_mpi_library_init(&prng);
  assert(cf_err == E_NOERR);
  
  cf_err = arith_circ_parse(&arith, params->arithfile);
  if(cf_err != E_NOERR)
    goto cleanup;
  
  cf_mpi_random_nbits(&mod, params->nbits, prng);

  n_inputs = arith_circ_n_inputs(arith);
  
  // generate messages
  cf_messages_random(&msgs, n_inputs, mod, prng);

  START_MEASURE(native_eval);
  cf_err = arith_circ_eval_pinocchio(&msgeval, arith, msgs, n_inputs, mod);
  END_MEASURE(native_eval);
  PRINT_MEASURE("pinocchio eval", native_eval);
  if(cf_err != E_NOERR)
    goto cleanup;
  
  //printf("eval message: ");
  //dump_messages(msgs, 1);
  if(params->verbose) {
    LOG_DUMP(cf_messages_dump(msgs, n_inputs), "messages: ");
    LOG_DUMP(cf_message_dump(msgeval), "result: ");
    LOG_DUMP(cf_mpi_dump(mod), "modulo: ");
    
  }
  
cleanup:
  cf_message_free(msgeval);
  cf_messages_free(msgs, n_inputs);
  arith_circ_free(arith);
  cf_mpi_free(mod);
  cf_mpi_library_free(prng);
  
  return cf_err;
}

cf_errno test_eval_native(test_params_t *params) {
  cf_errno cf_err = E_NOERR;
  arith_circ_t arith = NULL;
  cf_randstate_t prng = NULL;
  size_t n_inputs = 0;
  cf_message_t *msgs = NULL;
  cf_message_t msgeval = NULL, mod = NULL;
  
  cf_err = cf_mpi_library_init(&prng);
  assert(cf_err == E_NOERR);
  
  cf_err = arith_circ_parse(&arith, params->arithfile);
  if(cf_err != E_NOERR)
    goto cleanup;
  
  cf_mpi_random_nbits(&mod, params->nbits, prng);

  n_inputs = arith_circ_n_inputs(arith);
  
  // generate messages
  cf_messages_random(&msgs, n_inputs, mod, prng);

  START_MEASURE(native_eval);
  cf_err = arith_circ_eval(&msgeval, arith, msgs, n_inputs, mod);
  END_MEASURE(native_eval);
  PRINT_MEASURE("native eval", native_eval);
  if(cf_err != E_NOERR)
    goto cleanup;
  
  //printf("eval message: ");
  //dump_messages(msgs, 1);
  if(params->verbose) {
    LOG_DUMP(cf_messages_dump(msgs, n_inputs), "messages: ");
    LOG_DUMP(cf_message_dump(msgeval), "result: ");
    LOG_DUMP(cf_mpi_dump(mod), "modulo: ");
    
  }
  
cleanup:
  cf_message_free(msgeval);
  cf_messages_free(msgs, n_inputs);
  arith_circ_free(arith);
  cf_mpi_free(mod);
  cf_mpi_library_free(prng);
  
  return cf_err;
}
