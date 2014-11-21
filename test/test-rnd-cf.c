/**
 * Read a arith file, then generates messages and labels, and tests the CF scheme
 */
#include "test.h"
#include "cf-random.h"
#include "keys.h"

static int N = 10;
static FILE *outfile = NULL;

cf_errno test_rnd_cf(test_params_t *params) {
  cf_errno cf_err = E_NOERR;
  arith_circ_t arith = NULL;
  cf_keys_t keys = NULL;
  cf_message_t *msgs = NULL, msg_eval = NULL;
  cf_label_t *lbls = NULL;
  cf_tag_t *tags = NULL, tag_eval = NULL, local_tag = NULL;
  prf_hd_t prf = NULL;
  cf_program_t prgm = NULL;
  size_t n_inputs = 0, i;
  int n_test = 0;
  cf_randstate_t prng = NULL;
  
  if(params->outfile != NULL) {
    outfile = fopen(params->outfile,"w");
    assert(outfile != NULL);
    fprintf(outfile, "# test CF on circuit %s with %u bits\n", params->arithfile, params->nbits);
  }
  // parse circuit
  START_MEASURE(parse);
  if( (cf_err = arith_circ_parse(&arith, params->arithfile)) != E_NOERR)
    goto cleanup;
  END_MEASURE(parse);
  PRINT_MEASURE("parse", parse);
  if(params->outfile != NULL) {
    if(params->time_measure) {
      fprintf(outfile, "# parse = ");
      FPRINT_WALL_TIME(outfile, parse);
      fprintf(outfile, "\n");
      fprintf(outfile, "msgeval\tkeygen\tauth\teval\tvrfy\t\n");
    }
  }
  n_inputs = arith_circ_n_inputs(arith);
  // init cf
  if( (cf_err = cf_mpi_library_init(&prng)) != E_NOERR)
    goto cleanup;
    
  // test CF N times
  for(n_test = 0; n_test < N; n_test++) {
    // generate keys
    START_MEASURE(keygen);
    if( (cf_err = cf_key_gen(&keys, params->nbits, params->d, params->scheme, prng)) != E_NOERR)
      goto cleanup;
    END_MEASURE(keygen);
    PRINT_MEASURE("keygen", keygen);
    // generate messages
    cf_messages_random(&msgs, n_inputs, keys->ek->p, prng);
    // generate labels
    cf_labels_random(&lbls, n_inputs, params->nbits, prng);
    LOG_DUMP(cf_messages_dump(msgs, n_inputs), "messages: ");
    LOG_DUMP(cf_labels_dump(lbls, n_inputs), "labels: ");
    
    // evaluate plain messages
    START_MEASURE(msgeval);
    if( (cf_err = arith_circ_eval(&msg_eval, arith, msgs, n_inputs, keys->ek->p)) != E_NOERR)
      goto cleanup;
    END_MEASURE(msgeval);
    PRINT_MEASURE("msgeval", msgeval);
    // create prf handler
    
    if( (cf_err = prf_create_handler(&prf, keys->sk->k, params->nbits / 8)) != E_NOERR)
      goto cleanup;
    // execute cf_auth
    START_MEASURE(auth);
    cf_tags_new(&tags, n_inputs);
    for(i = 0; i < n_inputs; i++) {
      if( (cf_err = cf_auth(&(tags[i]), keys->sk, lbls[i], msgs[i], prf)) != E_NOERR)
        goto cleanup;
    }
    END_MEASURE(auth);
    PRINT_MEASURE("auth", auth);
    
    // execute cf_eval
    START_MEASURE(eval);
    if( (cf_err = cf_eval(&tag_eval, keys->ek, arith, tags, n_inputs, &local_tag, params->scheme)) != E_NOERR)
      goto cleanup;
    END_MEASURE(eval);
    PRINT_MEASURE("eval",eval);
    
    LOG_DUMP(cf_tag_dump(local_tag), "Local tag:");
    // execute cf_vrfy
    cf_program_init(&prgm, lbls, n_inputs, arith);
    START_MEASURE(vrfy);
    cf_err = cf_vrfy(keys->sk, msg_eval, prgm, tag_eval, prf);
    END_MEASURE(vrfy);
    PRINT_MEASURE("vrfy",vrfy);
    // VRFY OK
    if(cf_err == E_VRFY) {
      LOG_PRINT("vrfy ok\n");
      cf_err = E_NOERR;
    }
    
  if(outfile != NULL) {
    if(params->time_measure) {
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
    cf_keys_free(keys);
    cf_messages_free(msgs, n_inputs);
    cf_labels_free(lbls, n_inputs);
    cf_tags_free(tags, n_inputs);
    cf_message_free(msg_eval);
    cf_tag_free(tag_eval);
    cf_tag_free(local_tag);
    cf_program_free(prgm);
    prf_close_handler(prf);
  }

  if(outfile != NULL) {
    fprintf(outfile, "# iterations: %d\n", N);
    if(cf_err != E_NOERR) {
      fprintf(outfile, "# test failed with code %d\n", cf_err);
    }
    fclose(outfile);
  }
  arith_circ_free(arith);
  cf_mpi_library_free(prng);
  
  return cf_err;
}
