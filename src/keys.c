#include "keys.h"
#include "logger.h"

cf_errno cf_key_gen(cf_keys_t *keys, uint lambda, uint d, cf_scheme_e cf,
                    cf_randstate_t prng) {
  cf_errno cf_err = E_NOERR;
  cf_key_ek_t ek = NULL;
  cf_key_sk_t sk = NULL;
  cf_mpi_t q = NULL, t = NULL;
  size_t i = 0;
  
  if((lambda  % 8) != 0)
    return E_INVAL_SECPARAM;
  if(cf == CF_2 && d == 0) {
    return E_INVAL_DEGREE;
  }
  // allocate space for evaluation and secret keys
  ek = (cf_key_ek_t)malloc(sizeof(struct cf_key_ek_s));
  sk = (cf_key_sk_t)malloc(sizeof(struct cf_key_sk_s));
  assert(ek != NULL && sk != NULL);
  // set params
  ek->lambda = lambda;
  ek->d = d;
  EK_H(ek) = NULL;
  SK_G(sk) = NULL;
  
  switch(cf) {
    case CF_1: // just generate a random prime p of lambda bits
      cf_err = cf_mpi_random_prime(&(EK_P(ek)), lambda, prng);
      if(cf_err != E_NOERR)
        goto cleanup;
      LOG_MPI(EK_P(ek), "[keygen] p = ");
      break;
    case CF_2:
      // generate p and q primes, such that q = 2p + 1
      cf_mpi_init(&(EK_P(ek))); // init ek->p
      do {
        // generate a random prime of lambda-1 bits
        cf_err = cf_mpi_random_prime_no_alloc(&(EK_P(ek)), lambda - 1, prng);
        if(cf_err != E_NOERR)
          goto cleanup;
        cf_mpi_mul_ui(&q, EK_P(ek), 0x2); // q = 2p
        cf_mpi_add_ui(&q, q, 0x1);        // q = q+1
      } while(! cf_mpi_probab_prime(q));
        
      LOG_MPI(EK_P(ek), "[keygen] p = ");
      LOG_MPI(q, "[keygen] q = ");
      
      // find a generator of the group G using Schnorr group
      // see https://en.wikipedia.org/wiki/Schnorr_group
      cf_mpi_init(&(SK_G(sk)));
      cf_mpi_init(&t);
      do {
        // try a random t in [2, q-1]
        cf_mpi_random_mod_no_alloc(&t, q, prng);
        assert(cf_mpi_cmp_ui(t, 2UL) > 0);
        // g = h^2 mod q
        cf_mpi_powm_ui_no_alloc(&(SK_G(sk)), t, 2UL, q);
      } while(cf_mpi_cmp_ui(SK_G(sk), 1UL) == 0); // g != 1 mod q
      LOG_MPI(SK_G(sk), "[keygen] g = ");
      break;
    default:
      cf_err = E_NOT_IMPL;
      goto cleanup;
  }
  cf_mpi_copy(&(SK_P(sk)), EK_P(ek)); // copy ek->p to sk->p
  // choose a seed k of lambda bits
  cf_mpi_random_nbits_sec(&(SK_K(sk)), lambda, prng);
  assert(cf_mpi_nbits(SK_K(sk)) == lambda);
  LOG_MPI(SK_K(sk), "[keygen] k = ");
  
  // generate the random x in Z_p
  cf_mpi_random_mod_sec(&(SK_X(sk)), EK_P(ek), prng);
  assert(cf_mpi_cmp(SK_X(sk), EK_P(ek)) < 0);
  LOG_MPI(SK_X(sk), "[keygen] x = ");
  
  if(cf == CF_2) {
    // generate the h_i for i = 1 to d
    EK_H(ek) = (cf_mpi_t*)calloc(d, sizeof(cf_mpi_t));
    assert(EK_H(ek) != NULL);
    LOG_PRINT("[keygen] h = [\n");
    for(i = 1; i <= d; i++) {
      cf_mpi_init(&(EK_H(ek)[i-1]));
      cf_mpi_pow_ui(&(EK_H(ek)[i-1]), SK_X(sk), i);
      cf_mpi_powm(&(EK_H(ek)[i-1]), SK_G(sk), EK_H(ek)[i-1], q);
      LOG_MPI(EK_H(ek)[i-1], "  ");
    }
    LOG_PRINT("]\n");
  }
  
  if(cf_err == E_NOERR) {
    cf_err = cf_keys_new(keys, sk, ek);
  
    assert((*keys)->lambda == lambda);
    assert(ek->d == d);
  }
  
cleanup:
  cf_mpi_free(q);
  cf_mpi_free(t);
  if(cf_err != E_NOERR) {
    cf_key_ek_free(ek);
    cf_key_sk_free(sk);
  }
  
  return cf_err;
}

cf_errno cf_keys_new(cf_keys_t *keys, cf_key_sk_t sk, cf_key_ek_t ek) {
  if(ek == NULL) return E_NO_EK_KEY;
  if(sk == NULL) return E_NO_SK_KEY;
  *keys = malloc(sizeof(struct cf_keys_s));
  if(*keys == NULL)
    return E_NO_MEM;
  (*keys)->ek = ek;
  (*keys)->sk = sk;
  (*keys)->lambda = cf_mpi_nbits(sk->k);
  return E_NOERR;
}

/**
 * @brief Release the resources used by the evaluation key.
 * 
 * If the evaluation key is \c NULL, nothing is done.
 * 
 * @param[in] ek  Evaluation key.
 */
void cf_key_ek_free(cf_key_ek_t ek) {
  size_t i = 0;
  if(ek != NULL) {
    cf_mpi_free(ek->p);
    ek->p = NULL;
    if(ek->h != NULL) {
      for(i = 0; i < ek->d; i++) {
        cf_mpi_free(ek->h[i]);
        ek->h[i] = NULL;
      }
      free(ek->h);
      ek->h = NULL;
    }
    free(ek);
    ek = NULL;
  }
}

/**
 * @brief Release the resources used by the secret key.
 * 
 * If the secret key is \c NULL, nothing is done.
 * 
 * @param[in] sk  Secret key.
 */
void cf_key_sk_free(cf_key_sk_t sk) {
  if(sk != NULL) {
    cf_mpi_free(sk->k);
    sk->k = NULL;
    cf_mpi_free(sk->x);
    sk->x = NULL;
    cf_mpi_free(sk->p);
    sk->p = NULL;
    cf_mpi_free(sk->g);
    sk->g = NULL;
    free(sk);
    sk = NULL;
  }
}

void cf_keys_free(cf_keys_t k) {
  if(k != NULL) {
    cf_key_ek_free(k->ek);
    cf_key_sk_free(k->sk);
    free(k);
    k = NULL;
  }
}
