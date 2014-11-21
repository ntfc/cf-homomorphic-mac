#pragma once

#include "mpi-cf.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EK_P(ek)  ((ek)->p)
#define EK_H(ek)  ((ek)->h)
#define SK_K(sk)  ((sk)->k)
#define SK_P(sk)  ((sk)->p)
#define SK_X(sk)  ((sk)->x)
#define SK_G(sk)  ((sk)->g)

//! @brief Structure to hold a secret key.
struct cf_key_sk_s {
  //! @brief Seed of a PRF \f$F_k\colon \{0,1\}^* \to \mathbb{Z}_p\f$ of
  //! \f$\lambda\f$ bits.
  //! @note Used for #CF_1 and #CF_2.
  cf_mpi_t k;
  //! @brief Prime number of \f$\lambda\f$ bits.
  //! @note Used for #CF_1 and #CF_2.
  cf_mpi_t p;
  //! @brief Random value in \f$\mathbb{Z}_p\f$.
  //! @note Used for #CF_1 and #CF_2.
  cf_mpi_t x;
  //! @brief Random generator of a group \f$\mathbb{G}\f$ of order #p. 
  //! @note Used only for #CF_2.
  cf_mpi_t g;
};
//! @brief Type of a secret key.
typedef struct cf_key_sk_s *cf_key_sk_t;
//! @brief Structure to hold an evaluation key.
struct cf_key_ek_s {
  //! @brief Security parameter in bits.
  //! @note Used for #CF_1 and #CF_2.
  uint lambda;
  //! @brief Prime number of \f$\lambda\f$ bits
  //! @note Used for #CF_1 and #CF_2.
  cf_mpi_t p;
  //! @brief Upper bound on the degree of supported arithmetic circuits.
  //! @note Used only for #CF_2.
  uint d;
  //! @brief For \f$i=1\f$ to #d, \f$h[i] = {g^x}^i\f$.
  //! @note Used only for #CF_2.
  cf_mpi_t *h;
};
//! @brief Type of an evaluation key.
typedef struct cf_key_ek_s *cf_key_ek_t;
//! @brief Structure to hold a key pair with a secret key and an evaluation key.
struct cf_keys_s {
  //! @brief Security parameter in bits
  //! @todo Removed from here.
  uint lambda;
  //! @brief Previously generated evaluation key.
  cf_key_ek_t ek;
  //! @brief Previously generated secret key.
  cf_key_sk_t sk;
};
//! @brief Type of a key pair
typedef struct cf_keys_s *cf_keys_t;

/**
 * @brief Generates a key pair consisting of the evaluation key and secret key.
 * 
 * In case of error, all allocated resources are properly released.
 * 
 * @param[out] keys   Pointer to where the key pair should be stored.
 * @param[in]  lambda Security parameter in bits.
 * @param[in]  d      Bound on the degree of the circuits that can be evaluated.
 * @param[in]  cf     CF scheme to use.
 * 
 * @retval #E_NOERR    No errors occurred.
 * @retval #E_SECPARAM Invalid value for security parameter.
 * @retval #E_NO_MEM   Out of memory.
 * 
 */
cf_errno      cf_key_gen(cf_keys_t *keys, uint lambda, uint d, cf_scheme_e cf,
                         cf_randstate_t prng);

/**
 * @brief Given an already read pair of keys, create the respective key pair.
 * 
 * This functions is mostly intended to be used by the internal of
 * cf_key_gen(). It is not expected that this function might be used anywhere
 * else except there. In case of error, all allocated resources are properly
 *  released.
 * 
 * @param[out]  keys Pointer where the key pair should be stored.
 * @param[in]   sk   Secret key, previously allocated and generated.
 * @param[in]   ek   Evaluation key, previously allocated and generated.
 * 
 * @retval #E_NOERR     No errors occurred.
 * @retval #E_NO_EK_KEY No evaluation key.
 * @retval #E_NO_SK_KEY No secret key.
 * @retval #E_NO_MEM    Out of memory.
 */
cf_errno      cf_keys_new(cf_keys_t *keys, cf_key_sk_t sk, cf_key_ek_t ek);

/**
 * @brief Release the resources used by the evaluation key.
 * 
 * If the evaluation key is \c NULL, nothing is done.
 * 
 * @param[in] ek  Evaluation key.
 */
void          cf_key_ek_free(cf_key_ek_t ek);

/**
 * @brief Release the resources used by the secret key.
 * 
 * If the secret key is \c NULL, nothing is done.
 * 
 * @param[in] sk  Secret key.
 */
void          cf_key_sk_free(cf_key_sk_t sk);
/**
 * @brief Release the resources used by the key pair.
 * 
 * If the key pair is \c NULL, nothing is done.
 * 
 * @param[in] k  Key pair.
 */
void          cf_keys_free(cf_keys_t k);


#ifdef __cplusplus
}
#endif
