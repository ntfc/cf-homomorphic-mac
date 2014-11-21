#include "cf.h"
#include "logger.h"
#include "timemeasure.h"

//! @todo Should this be elsewhere?
int is_verbose = 0;

cf_errno cf_auth(cf_tag_t *tag, cf_key_sk_t sk, cf_label_t label, cf_message_t message, prf_hd_t prf) {
  cf_errno cf_err = E_NOERR;
  uint lambda;
  cf_mpi_t tmp = NULL, r_tau = NULL;

  if(!sk) return E_NO_SK_KEY;
  if(!label) return E_INVAL_LBL;
  if(!message) return E_INVAL_MSG;
  
  lambda = cf_mpi_nbits(SK_K(sk));

  if((lambda % 8) != 0) return E_INVAL_SECPARAM;
  if(cf_mpi_cmp(message, SK_P(sk)) > 0) return E_INVAL_MSG;
  
  if(cf_mpi_nbits(label) > lambda) return E_INVAL_LBL;
  
  cf_err = cf_prf_encrypt_label(&r_tau, sk, label, prf);
  
  if(cf_err == E_NOERR) {
    // create the tag (y0,y1)
    cf_tag_new(tag, 1);
    cf_mpi_copy(&((*tag)->y[0]), message);
    
    cf_mpi_init2(&((*tag)->y[1]), cf_mpi_nbits(SK_P(sk)));
    cf_mpi_inv(&((*tag)->y[1]), SK_X(sk), SK_P(sk)); // x^-1 mod p
    assert((*tag)->y[1] != NULL);

    cf_mpi_init2(&tmp, cf_mpi_nbits(r_tau));
    cf_mpi_subm(&tmp, r_tau, message, SK_P(sk)); // (r_tau - m)
    cf_mpi_mulm(&((*tag)->y[1]), (*tag)->y[1], tmp, SK_P(sk));
    
    cf_mpi_free(tmp);
    cf_mpi_free(r_tau);

  }
  return cf_err;
}

cf_errno cf_eval(cf_tag_t *result, cf_key_ek_t key, const arith_circ_t arith,
                 const cf_tag_t *tags, size_t ntags, cf_tag_t *local_tag, cf_scheme_e cf) {
  cf_errno cf_err = E_NOERR;
  cf_tag_t aux = NULL;
  assert(result != local_tag);
  
  if(arith_circ_n_inputs(arith) != ntags) {
    return E_INVAL_N_INPUTS;
  }
  if(cf == CF_2 && key->h == NULL) {
    return E_INVAL_EK_KEY;
  }
  
  // TODO: get tag degree before evaluating..
  // returns a tag [y0, .., yd]
  cf_err = _cf_eval_tags(result, arith, tags, ntags, key->p, cf);
  if(cf == CF_2 && CF_TAG_DEGREE((*result)) > key->d) {
    cf_tag_free(*result);
    *result = NULL;
    
    return E_INVAL_DEGREE;
  }
  // do the CF2 compression
  if(cf == CF_2) {
    // swap values
    aux = *result;
    *result = *local_tag;
    *local_tag = aux;
    cf_tag_new(result, 0); // element of G
    _cf_compress_tag(result, key, *local_tag);
  }
  
  return cf_err;
}

cf_errno _cf_compress_tag(cf_tag_t *result, cf_key_ek_t key, cf_tag_t local_tag) {
  size_t i = 0;
  cf_errno cf_err = E_NOERR;
  cf_mpi_t q = NULL, t = NULL;
  
  cf_mpi_init(&t);
  cf_mpi_init(&((*result)->y[0]));
  // q = 2p + 1
  cf_mpi_mul_ui(&q, EK_P(key), 2UL);
  cf_mpi_add_ui(&q, q, 1UL);
  cf_mpi_set_ui(&((*result)->y[0]), 1UL);
  
  for(i = 1; i <= CF_TAG_DEGREE(local_tag); i++) {
    // t = h_{i-1}^{tag[i]} mod q
    cf_mpi_powm_no_alloc(&t, EK_H(key)[i - 1], local_tag->y[i], q);
    // result->y[0] *= t
    cf_mpi_mulm_no_alloc(&((*result)->y[0]), (*result)->y[0], t, q);
  }
  cf_mpi_free(q);
  cf_mpi_free(t);
  
  return cf_err;
}

static void _compute_rho(cf_mpi_t *rho, cf_key_sk_t sk, cf_tag_t tag) {
  size_t k;
  cf_mpi_t xk = NULL;
  
  cf_mpi_set_ui(rho, 0x0);
  
  for(k = 0; k < tag->n; k++) {
    cf_mpi_init2(&xk, cf_mpi_nbits(SK_X(sk)));
    
    cf_mpi_powm_ui(&xk, SK_X(sk), k, SK_P(sk));
    cf_mpi_mulm(&xk, xk, tag->y[k], SK_P(sk));
    cf_mpi_addm(rho, *rho, xk, SK_P(sk));
    cf_mpi_free(xk);
  }
}

static cf_errno _compute_rho_eval(cf_mpi_t *rho, cf_key_sk_t sk, cf_program_t program, prf_hd_t prf) {
  cf_errno cf_err;
  size_t n_inputs = cf_program_n_inputs(program);
  cf_mpi_t r_tau[n_inputs];
  size_t i;
  // for every input wire of f with label tau, compute F_k(tau)
  for(i = 0; i < n_inputs; i++) {
    cf_err = cf_prf_encrypt_label(&(r_tau[i]), sk, (cf_program_input_lbls(program))[i], prf);
    assert(cf_err == E_NOERR);
  }
  // evaluate the circuit, using the r_tau as input
  cf_err = arith_circ_eval(rho, cf_program_arith_circuit(program), r_tau, n_inputs, SK_P(sk));
  
  for(i = 0; i < n_inputs; i++) {
   // gcry_mpi_release(r_tau[i]);
    cf_mpi_free(r_tau[i]);
  }
  return cf_err;
}

cf_errno cf_vrfy(cf_key_sk_t sk, cf_message_t message, cf_program_t program, cf_tag_t tag, prf_hd_t prf) {
  cf_errno cf_err = E_NO_VRFY;
  uint lambda;
  cf_mpi_t rho_eval = NULL, rho = NULL;
  int close_prf = 0;
  cf_mpi_t q = NULL;
  cf_scheme_e cf = CF_1;
  cf_mpi_t g_rho = NULL, g_m = NULL;
  
  if(sk == NULL) return E_NO_SK_KEY;
  if(message == NULL) return E_NO_MSG;
  if(tag == NULL) return E_NO_TAG;
  if(program == NULL) return E_NO_PRGM;
  if(cf_mpi_cmp(message, SK_P(sk)) > 0) return E_INVAL_MSG;
  
  if(CF_TAG_DEGREE(tag) == 0) {
    if(tag->y[0] == NULL)
      return E_INVAL_TAG;
    cf = CF_2;
  }
  lambda = cf_mpi_nbits(SK_K(sk));
  if((lambda % 8) != 0) return E_INVAL_SECPARAM; 

  if(prf == NULL) {
    close_prf = 1;
    START_MEASURE(create_prf);
    cf_err = prf_create_handler(&prf, SK_K(sk), lambda / 8);
    if(is_verbose) {
      END_MEASURE(create_prf);
      LOG_PRINT("[vrfy] ");
      PRINT_MEASURE("prf creation", create_prf);
    }
    assert(cf_err == E_NOERR);
  }
  if(cf == CF_1) {
    // compare y0 and m
    if(cf_mpi_cmp(tag->y[0], message) == 0) {
      START_MEASURE(rho_eval_time);
      cf_err = _compute_rho_eval(&rho_eval, sk, program, prf);
      if(is_verbose) {
        END_MEASURE(rho_eval_time);
        LOG_PRINT("[vrfy] ");
        PRINT_MEASURE("rho eval", rho_eval_time);
      }
      assert(cf_err == E_NOERR);
      LOG_MPI(rho_eval, "rho(eval) = ");
      
      // compute rho
      START_MEASURE(rho_time);
      _compute_rho(&rho, sk, tag);
      if(is_verbose) {
        END_MEASURE(rho_time);
        LOG_PRINT("[vrfy] ");
        PRINT_MEASURE("rho", rho_time);
      }
      LOG_MPI(rho, "rho = ");
      START_MEASURE(rho_cmp);
      if(cf_mpi_cmp(rho, rho_eval) == 0) {
        cf_err = E_VRFY;
      }
      if(is_verbose) {
        END_MEASURE(rho_cmp);
        LOG_PRINT("[vrfy] ");
        PRINT_MEASURE("rho cmp", rho_cmp);
      }
      // clean everything..
      cf_mpi_free(rho);
      cf_mpi_free(rho_eval);
    }
  }
  else {
    // q = 2p + 1
    cf_mpi_mul_ui(&q, SK_P(sk), 2UL);
    cf_mpi_add_ui(&q, q, 1UL);
    
    START_MEASURE(rho_eval_time);
    cf_err = _compute_rho_eval(&rho_eval, sk, program, prf);
    if(is_verbose) {
      END_MEASURE(rho_eval_time);
      PRINT_MEASURE("[vrfy] rho eval", rho_eval_time);
    }
    assert(cf_err == E_NOERR);
    LOG_MPI(rho_eval, "rho(eval) = ");
  
    cf_mpi_init(&g_rho);
    cf_mpi_init(&g_m);
    
    START_MEASURE(compars);
    cf_mpi_powm(&g_rho, SK_G(sk), rho_eval, q);
    cf_mpi_powm(&g_m, SK_G(sk), message, q);
    cf_mpi_mulm_no_alloc(&g_m, g_m, tag->y[0], q);
    
    cf_err = cf_mpi_cmp(g_rho, g_m) == 0 ? E_VRFY : E_NO_VRFY;
    if(is_verbose) {
      END_MEASURE(compars);
      PRINT_MEASURE("[vrfy] comparations", compars);
    }
    
    cf_mpi_free(g_rho);
    cf_mpi_free(g_m);
    cf_mpi_free(q);
  }
  if(close_prf == 1)
    prf_close_handler(prf);
  return cf_err;
}

cf_errno cf_prf_encrypt_label(cf_mpi_t *r, const cf_key_sk_t sk, cf_label_t lbl,
                              prf_hd_t prf) {
  cf_errno cf_err = E_NOERR;
  if(!sk) return E_NO_SK_KEY;
  if(!lbl) return E_NO_LBL;
  
  *r = NULL;
  cf_err = prf_encrypt_mpi(r, SK_K(sk), SK_P(sk), lbl, prf);
  return cf_err;
}
