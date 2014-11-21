/*
 * If argv[1] is one of the functions available in CF protocol, execute the
 * arpg_FUNCTION parser. This parser verifies that all necessary arguments are
 * set. The validation of those arguments is sometimes done inside CF itself.
 * 
 */
#include "main.h"
#include "timemeasure.h"
#include "logger.h"

int main(int argc, char **argv) {
  struct arguments arguments = {
    .function = NULL, .verbose = 0, .outfile = NULL, .labelfile = NULL,
    .arithfile = NULL, .progfile = NULL, .tagfile = NULL, .messagefile = NULL,
    .skfile = NULL, .ekfile = NULL, .secparam = 128, .time_measure = 0,
    .d = 0, .scheme = CF_1
  };
  // by default, verbose and time_measure are OFF
  VERBOSE_OFF; TIME_MEASURE_OFF;
  
  // when calling argp_*(), we do --argc and ++argv to change the name of the 
  // argp program to KEYGEN, AUTH, etc
  if(argc > 1) {
    // the first argument must be one of the allowed CF commands
    if(strncmp(argv[1], KEYGEN, strlen(argv[1])) == 0) {
      ///////////////////////////////////////////////////////////////////////////
      // KEYGEN
      ///////////////////////////////////////////////////////////////////////////
      arguments.function = KEYGEN;
      argp_parse(&argp_keygen, --argc, ++argv, 0, 0, &arguments);
    }
    else if(strncmp(argv[1], AUTH, strlen(argv[1])) == 0) {
      ///////////////////////////////////////////////////////////////////////////
      // AUTH
      ///////////////////////////////////////////////////////////////////////////
      arguments.function = AUTH;
      argp_parse(&argp_auth, --argc, ++argv, 0, 0, &arguments);
    }
    else if(strncmp(argv[1], EVAL, strlen(argv[1])) == 0) {
      ///////////////////////////////////////////////////////////////////////////
      // EVAL
      ///////////////////////////////////////////////////////////////////////////
      arguments.function = EVAL;
      argp_parse(&argp_eval, --argc, ++argv, 0, 0, &arguments);
    }
    else if(strncmp(argv[1], VRFY, strlen(argv[1])) == 0) {
      ///////////////////////////////////////////////////////////////////////////
      // VRFY
      ///////////////////////////////////////////////////////////////////////////
      arguments.function = VRFY;
      argp_parse(&argp_vrfy, --argc, ++argv, 0, 0, &arguments);
    }
    else if(strncmp(argv[1], MSGEVAL, strlen(argv[1])) == 0) {
      ///////////////////////////////////////////////////////////////////////////
      // MSGEVAL
      ///////////////////////////////////////////////////////////////////////////
      arguments.function = MSGEVAL;
      argp_parse(&argp_msgeval, --argc, ++argv, 0, 0, &arguments);
    }
  }
  // no valid FUNCTION was choose, so just show help
  if(arguments.function == NULL) {
    // show a simple help with the available options
    argp_help(&argp_global, stderr, 
      ARGP_HELP_SHORT_USAGE|ARGP_HELP_LONG|ARGP_HELP_POST_DOC|ARGP_HELP_EXIT_ERR,
      "cf");
  }
  
  return E_NOERR;
}

#pragma GCC diagnostic push // start of GCC ignore calls
#pragma GCC diagnostic ignored "-Wunused-parameter"
//#pragma GCC diagnostic ignored "-Wunused-variable"
static inline cf_errno parse_opt(int key, char *arg, struct argp_state *state) {
  switch(key) {
    case ARGP_KEY_FINI:
      argp_state_help(state, stderr, ARGP_HELP_SHORT_USAGE|ARGP_HELP_LONG|ARGP_HELP_POST_DOC|ARGP_HELP_EXIT_ERR);
      break;
  }
  return E_NOERR;
}
#pragma GCC diagnostic pop // stop of GCC ignore calls

static cf_errno parse_opt_keygen(int key, char *arg, struct argp_state *state) {
  cf_errno cf_err = E_NOERR;
  struct arguments *args = state->input;
  cf_keys_t keys = NULL;
  cf_randstate_t prng = NULL;

  switch(key) {
    case 'n':
      if( (cf_err = str_to_uint(&(args->secparam), arg, 10)) != E_NOERR)
        argp_error(state, "%s", str_cferror(cf_err));
      break;
    case 'o':
      args->outfile = arg; break;
    case 'c':
      if(strlen(arg) == 1 && (arg[0] == '1' || arg[0] == '2')) {
        switch(arg[0]) {
          case '1': args->scheme = CF_1; break;
          case '2': args->scheme = CF_2; break;
        }
      }
      else {
        argp_error(state, "%s (expected scheme to be 1 or 2)", str_cferror(E_INVAL_NUM));
      }
      break;
    case 'D':
      if( (cf_err = str_to_uint(&(args->d), arg, 10)) != E_NOERR) {
        argp_error(state, "%s", str_cferror(cf_err));
      }
      break;
    case 305: args->time_measure = 1; TIME_MEASURE_ON; break;
    case 'v': args->verbose = 1; VERBOSE_ON; break;
    case ARGP_KEY_ARG:
      if(strncmp(arg, args->function, strlen(arg)) != 0) {
        argp_failure(state, 1, 0, "too many FUNCTIONs");
      }
      break;
    case ARGP_KEY_FINI: // finished parsing
      if(args->scheme == CF_2 && args->d == 0) {
        argp_error(state, "Invalid D parameter (must be >0)");
      }
      
      LOG_PRINT("[keygen] %u bits, d = %u, scheme = %d..\n", args->secparam, args->d, args->scheme);
      if( (cf_err = cf_mpi_library_init(&prng)) == E_NOERR) {
        LOG_PRINT("[keygen] PRNG created..\n");
        // keygen
        cf_err = cf_key_gen(&keys, args->secparam, args->d, args->scheme, prng);
        LOG_DUMP(cf_keys_dump(keys), "[keygen] Dumping keys..\n");
        cf_mpi_library_free(prng);
        
      }
      if(cf_err == E_NOERR) {
        LOG_PRINT("[keygen] key pair created. Writing to out..\n");
        if( (cf_err = cf_keys_write(args->outfile, keys)) != E_NOERR) {
          cf_perror(args->outfile, cf_err);
        }
      }
      else {
        cf_perror("keygen", cf_err);
      }
      cf_keys_free(keys);
      break;
  }
  return cf_err;
}

static cf_errno parse_opt_auth(int key, char *arg, struct argp_state *state) {
  cf_errno cf_err = E_NOERR, err1 = E_NOERR, err2 = E_NOERR, err3 = E_NOERR;
  struct arguments *args = state->input; 
  cf_key_sk_t sk = NULL;
  cf_label_t *lbls = NULL;
  cf_message_t *msgs = NULL;
  cf_tag_t *tags = NULL;
  size_t n_lbls = 0, n_msgs = 0, i;
  prf_hd_t prf = NULL;
  
  switch(key) {
    case 's':
      args->skfile = arg; break;
    case 'l':
      args->labelfile = arg; break;
    case 'm':
      args->messagefile = arg; break;
    case 'o':
      args->outfile = arg; break;
    case 305: args->time_measure = 1; TIME_MEASURE_ON; break;
    case 'v': args->verbose = 1; VERBOSE_ON; break;
    case ARGP_KEY_ARG:
      if(strncmp(arg, args->function, strlen(arg)) != 0) {
        argp_failure(state, 1, 0, "too many FUNCTIONs");
      }
      break;
    case ARGP_KEY_FINI: // finished parsing
      if(args->skfile == NULL) cf_err = E_NO_SK_KEY;
      else if(args->labelfile == NULL) cf_err = E_NO_LBL;
      else if(args->messagefile == NULL) cf_err = E_NO_MSG;
      if(cf_err != E_NOERR) {
        argp_error(state, "%s", str_cferror(cf_err));
      }

      // read sk key
      if( (err1 = cf_key_sk_read(&sk, args->skfile)) == E_NOERR &&
          (err2 = cf_labels_read(&lbls, &n_lbls, args->labelfile)) == E_NOERR &&
          (err3 = cf_messages_read(&msgs, &n_msgs, args->messagefile)) == E_NOERR) {
        // set the security parameter from key
        args->secparam = cf_mpi_nbits(SK_K(sk));
        if(n_lbls != n_msgs) {
          cf_err = E_MSG_LBL_SIZE;
        }
        cf_err = prf_create_handler(&prf, SK_K(sk), args->secparam / 8);
        
        if(cf_err == E_NOERR) {
          // everything ready for auth
          cf_tags_new(&tags, n_msgs);
          for(i = 0; i < n_msgs; i++) {
            cf_err = cf_auth(&(tags[i]), sk, lbls[i], msgs[i], prf);
            if(cf_err != E_NOERR) {
              LOG_PRINT("[auth] failed to auth message %zu\n", i);
              break;
            }
          }
        }
        // write result
        if(cf_err == E_NOERR) {
          // write result
          if((cf_err = cf_tags_write(args->outfile, tags, n_msgs)) != E_NOERR)
            cf_perror(args->outfile, cf_err);
        }
        else
          cf_perror("auth", cf_err);
      }
      else {
        if(err1 != E_NOERR) cf_perror(args->skfile, err1);
        if(err2 != E_NOERR) cf_perror(args->labelfile, err2);
        if(err3 != E_NOERR) cf_perror(args->messagefile, err3);
      }
      
      // clean up
      cf_messages_free(msgs, n_msgs);
      cf_labels_free(lbls, n_lbls);
      cf_tags_free(tags, n_msgs);
      cf_key_sk_free(sk);
      prf_close_handler(prf);
      break;
  }
  
  return E_NOERR;
}

static cf_errno parse_opt_eval(int key, char *arg, struct argp_state *state) {
  cf_errno cf_err = E_NOERR, err1 = E_NOERR, err2 = E_NOERR, err3 = E_NOERR;
  struct arguments *args = state->input;
  cf_key_ek_t ek = NULL;
  cf_tag_t eval_tag = NULL, *tags = NULL, local_tag = NULL;
  size_t n_tags = 0;
  arith_circ_t arith = NULL;
  char *local_tag_fn = NULL;
  
  switch(key) {
    case 'e':
      args->ekfile = arg; break;
    case 'a':
      args->arithfile = arg; break;
    case 't':
      args->tagfile = arg; break;
    case 'c':
      if(strlen(arg) == 1 && (arg[0] == '1' || arg[0] == '2')) {
        switch(arg[0]) {
          case '1': args->scheme = CF_1; break;
          case '2': args->scheme = CF_2; break;
        }
      }
      else {
        argp_error(state, "%s (expected scheme to be 1 or 2)", str_cferror(E_INVAL_NUM));
      }
      break;
    case 'o':
      args->outfile = arg; break;
    case 305: args->time_measure = 1; TIME_MEASURE_ON; break;
    case 'v': args->verbose = 1; VERBOSE_ON; break;
    case ARGP_KEY_ARG:
      if(strncmp(arg, args->function, strlen(arg)) != 0) {
        argp_failure(state, 1, 0, "too many FUNCTIONs");
      }
      break;
    case ARGP_KEY_FINI: // finished parsing
      if(args->ekfile == NULL) cf_err = E_NO_EK_KEY;
      else if(args->arithfile == NULL) cf_err = E_NO_ARITH;
      else if(args->tagfile == NULL) cf_err = E_NO_TAG;
      if(cf_err != E_NOERR) {
        argp_error(state, "%s", str_cferror(cf_err));
      }
      
      LOG_PRINT("[eval] parsing evaluation key\n");
      if( (err1 = cf_key_ek_read(&ek, args->ekfile)) != E_NOERR)
        goto cleanup;
      LOG_PRINT("[eval] parsing arithmetic circuit\n");
      if( (err2 = arith_circ_parse(&arith, args->arithfile)) != E_NOERR)
        goto cleanup;
      LOG_PRINT("[eval] parsing tags\n");
      if( (err3 = cf_tags_read(&tags, &n_tags, args->tagfile)) != E_NOERR)
        goto cleanup;
      
      LOG_PRINT("[eval] homomorphic evaluation\n");
      START_MEASURE(eval);
      cf_err = cf_eval(&eval_tag, ek, arith, tags, n_tags, &local_tag, args->scheme);
      END_MEASURE(eval);
      PRINT_MEASURE("eval", eval);
      // write results
      if(args->outfile != NULL) {
        local_tag_fn = calloc(strlen(args->outfile) + 4, sizeof(char));
        strncpy(local_tag_fn, args->outfile, strlen(args->outfile));
        strncat(local_tag_fn, ".loc", 4);
      }
      
      if(cf_err == E_NOERR) {
        LOG_PRINT("[eval] writing evaluated tag\n");
        if((cf_err = cf_tag_write(args->outfile, eval_tag)) != E_NOERR) {
          cf_perror(args->outfile, cf_err);
          goto cleanup;
        }
        if(args->scheme == CF_2) {
          // write local tag
          if((cf_err = cf_tag_write(local_tag_fn, local_tag)) != E_NOERR) {
            cf_perror(local_tag_fn, cf_err);
            goto cleanup;
          }
        }
      }
      else {
        cf_perror("eval", cf_err);
      }
      
    cleanup:
      if(err1 != E_NOERR) cf_perror(args->ekfile, err1);
      if(err2 != E_NOERR) cf_perror(args->arithfile, err2);
      if(err3 != E_NOERR) cf_perror(args->tagfile, err3);
      // clean up
      cf_key_ek_free(ek);
      arith_circ_free(arith);
      cf_tags_free(tags, n_tags);
      cf_tag_free(eval_tag);
      cf_tag_free(local_tag);
      if(local_tag_fn != NULL)
        free(local_tag_fn);
      break;
  }
  return cf_err;
}

static cf_errno parse_opt_vrfy(int key, char *arg, struct argp_state *state) {
  cf_errno cf_err = E_NOERR, err1 = E_NOERR, err2 = E_NOERR, err3 = E_NOERR,
    err4 = E_NOERR, err5 = E_NOERR;
  struct arguments *args = state->input;
  cf_key_sk_t sk = NULL;
  arith_circ_t arith = NULL;
  cf_tag_t *tags = NULL;
  cf_label_t *lbls = NULL;
  cf_message_t *msgs = NULL, msg_eval;
  size_t n_lbls = 0, n_msgs = 0, n_tags = 0;
  cf_program_t prgm = NULL;
  switch(key) {
    case 's':
      args->skfile = arg; break;
    case 'a':
      args->arithfile = arg; break;
    case 'l':
      args->labelfile = arg; break;
    case 'm':
      args->messagefile = arg; break;
    case 't':
      args->tagfile = arg; break;
    case 'o':
      args->outfile = arg; break;
    case 305: args->time_measure = 1; TIME_MEASURE_ON; break;
    case 'v': args->verbose = 1; VERBOSE_ON; break;
    case ARGP_KEY_ARG:
      if(strncmp(arg, args->function, strlen(arg)) != 0) {
        argp_failure(state, 1, 0, "too many FUNCTIONs");
      }
      break;
    case ARGP_KEY_FINI: // finished parsing
      if(args->skfile == NULL) cf_err = E_NO_SK_KEY;
      else if(args->arithfile == NULL) cf_err = E_NO_ARITH;
      else if(args->labelfile == NULL) cf_err = E_NO_LBL;
      else if(args->messagefile == NULL) cf_err = E_NO_MSG;
      else if(args->tagfile == NULL) cf_err = E_NO_TAG;
      if(cf_err != E_NOERR) {
        argp_error(state, "%s", str_cferror(cf_err));
      }

      LOG_PRINT("[vrfy] parsing secret key\n");
      if( (err1 = cf_key_sk_read(&sk, args->skfile)) != E_NOERR)
        goto cleanup;
      LOG_PRINT("[vrfy] parsing arithmetic circuit\n");
      if( (err2 = arith_circ_parse(&arith, args->arithfile)) != E_NOERR)
        goto cleanup;
      LOG_PRINT("[vrfy] parsing labels\n");
      if( (err3 = cf_labels_read(&lbls, &n_lbls, args->labelfile)) != E_NOERR)
        goto cleanup;
      LOG_PRINT("[vrfy] parsing messages\n");
      if( (err4 = cf_messages_read(&msgs, &n_msgs, args->messagefile)) != E_NOERR)
        goto cleanup;
      LOG_PRINT("[vrfy] parsing tag\n");
      if( (err5 = cf_tags_read(&tags, &n_tags, args->tagfile)) != E_NOERR)
        goto cleanup;
      if(n_tags != 1)
        cf_err = E_TAG_SIZE;
      else {
        if(n_msgs == 1)
          cf_message_copy(&msg_eval, msgs[0]);
        else if(n_msgs == n_lbls)
          cf_err = arith_circ_eval(&msg_eval, arith, msgs, n_msgs, SK_P(sk));
        else
          cf_err = E_MSG_LBL_SIZE;
      }
      // ready for vrfy?
      if(cf_err == E_NOERR) {
        cf_program_init(&prgm, lbls, n_lbls, arith);
        LOG_PRINT("[vrfy] starting homomorphic verification\n");
        START_MEASURE(vrfy);
        cf_err = cf_vrfy(sk, msg_eval, prgm, tags[0], NULL);
        END_MEASURE(vrfy);
        PRINT_MEASURE("vrfy", vrfy);
      }
      if(cf_err != E_NOERR)
        cf_perror("vrfy", cf_err);
    cleanup:
      if(err1 != E_NOERR) cf_perror(args->skfile, err1);
      if(err2 != E_NOERR) cf_perror(args->arithfile, err2);
      if(err3 != E_NOERR) cf_perror(args->labelfile, err3);
      if(err4 != E_NOERR) cf_perror(args->messagefile, err4);
      if(err5 != E_NOERR) cf_perror(args->tagfile, err5);
      // clean up
      cf_key_sk_free(sk);
      cf_labels_free(lbls, n_lbls);
      cf_messages_free(msgs, n_msgs);
      cf_message_free(msg_eval);
      cf_tags_free(tags, n_tags);
      cf_program_free(prgm);
      arith_circ_free(arith);
      break;
  }

  return cf_err;
}

static cf_errno parse_opt_msgeval(int key, char *arg, struct argp_state *state) {
  cf_errno cf_err = E_NOERR, err1 = E_NOERR, err2 = E_NOERR;
  struct arguments *args = state->input;
  arith_circ_t arith = NULL;
  cf_message_t *msgs = NULL, msg_eval = NULL;
  size_t n_msgs = 0;
  cf_key_ek_t ek = NULL;
  
  switch(key) {
    case ARGP_KEY_INIT: break;
    case 'a':
      args->arithfile = arg; break;
    case 'm':
      args->messagefile = arg; break;
    case 'e':
      args->ekfile = arg; break;
    case 'o':
      args->outfile = arg; break;
    case 305: args->time_measure = 1; TIME_MEASURE_ON; break;
    case 'v': args->verbose = 1; VERBOSE_ON; break;
    case ARGP_KEY_FINI: // finished parsing
      if(args->arithfile == NULL) cf_err = E_NO_ARITH;
      else if(args->messagefile == NULL) cf_err = E_NO_MSG;
      if(cf_err != E_NOERR) {
        argp_error(state, "%s", str_cferror(cf_err));
      }

      if( (err1 = arith_circ_parse(&arith, args->arithfile)) != E_NOERR)
        goto cleanup;
      if( (err2 = cf_messages_read(&msgs, &n_msgs, args->messagefile)) != E_NOERR)
        goto cleanup;
      // read key if necessary
      
      if(args->ekfile != NULL) {
        cf_err = cf_key_ek_read(&ek, args->ekfile);
      }
      if(cf_err != E_NOERR) {
        cf_perror(args->ekfile, cf_err);
        goto cleanup;
      }
      else {
        // proceed with the evaluation
        if(ek != NULL)
          cf_err = arith_circ_eval(&msg_eval, arith, msgs, n_msgs, EK_P(ek));
        else
          cf_err = arith_circ_eval(&msg_eval, arith, msgs, n_msgs, NULL);
        if(cf_err == E_NOERR) {
          if(args->outfile != NULL) {
            FILE *fp = fopen(args->outfile, "w");
            cf_mpi_write(fp, msg_eval);
            fclose(fp);
          }
          else
            cf_message_dump(msg_eval);
          if(cf_err != E_NOERR)
            cf_perror(args->outfile, cf_err);
        }
        else
          cf_perror("msgeval", cf_err);
          goto cleanup;
      }
      // clean up
    cleanup:
      fprintf(stderr, "Cleaning\n");
      cf_message_free(msg_eval);
      cf_key_ek_free(ek);
      cf_messages_free(msgs, n_msgs);
      arith_circ_free(arith);
      if(err2 != E_NOERR) cf_perror(args->messagefile, err2);
      if(err1 != E_NOERR) cf_perror(args->arithfile, err1);
      break;
  }
  return cf_err;
}
