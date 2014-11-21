#pragma once

#include <gmp.h>
#include "error.h"

#ifdef __cplusplus
extern "C" {
#endif

//typedef mpz_t cf_mpi_t;
typedef struct cf_mpi_s {
  mpz_t mpi;
  uint8_t _alloced;
} *cf_mpi_t;

typedef struct cf_randstate_s {
  gmp_randstate_t state;
  uint8_t _alloced;
} *cf_randstate_t;

////////////////////////////////////////////
// CF MPI stuff
////////////////////////////////////////////

cf_errno  cf_mpi_library_init(cf_randstate_t *prng);
void      cf_mpi_library_free(cf_randstate_t prng);

void cf_mpi_init(cf_mpi_t *a);
void cf_mpi_init2(cf_mpi_t *a, uint nbits);
void cf_mpi_free(cf_mpi_t a);
void cf_mpi_copy(cf_mpi_t *dest, const cf_mpi_t orig);

#define cf_mpi_nbits(a) MPZ_NBITS((a)->mpi)

cf_errno cf_mpi_random_prime(cf_mpi_t *prime, uint nbits, cf_randstate_t prng);
cf_errno cf_mpi_random_prime_no_alloc(cf_mpi_t *prime, uint nbits, cf_randstate_t prng);
#define  cf_mpi_random_prime_sec(p, n, prng)  cf_mpi_random_prime(p, n, prng);
//cf_errno cf_mpi_random_prime_sec(cf_mpi_t *prime, uint nbits, cf_randstate_t prng);
void     cf_mpi_random_nbits(cf_mpi_t *r, uint nbits, cf_randstate_t prng);
#define  cf_mpi_random_nbits_sec(r, n, prng)  cf_mpi_random_nbits(r, n, prng);
//void     cf_mpi_random_nbits_sec(cf_mpi_t *r, uint nbits, cf_randstate_t prng);
void     cf_mpi_random_mod(cf_mpi_t *r, cf_mpi_t mod, cf_randstate_t prng);
void     cf_mpi_random_mod_no_alloc(cf_mpi_t *r, cf_mpi_t mod, cf_randstate_t prng);
#define  cf_mpi_random_mod_sec(r, m, prng)  cf_mpi_random_mod(r, m, prng);
//void     cf_mpi_random_mod_sec(cf_mpi_t *r, cf_mpi_t mod, cf_randstate_t prng);

#define cf_mpi_cmp(a, b) mpz_cmp((a)->mpi, (b)->mpi)
#define cf_mpi_cmp_ui(a, b) mpz_cmp_ui((a)->mpi, b)

int     cf_mpi_probab_prime(const cf_mpi_t);

void    cf_mpi_add(cf_mpi_t *r, const cf_mpi_t a, const cf_mpi_t b);
void    cf_mpi_add_ui(cf_mpi_t *r, const cf_mpi_t a, unsigned long int b);
void    cf_mpi_addm(cf_mpi_t *r, const cf_mpi_t a, const cf_mpi_t b, const cf_mpi_t mod);
void    cf_mpi_mul(cf_mpi_t *r, const cf_mpi_t a, const cf_mpi_t b);
void    cf_mpi_mul_ui(cf_mpi_t *r, const cf_mpi_t a, unsigned long int b);
void    cf_mpi_mulm(cf_mpi_t *r, const cf_mpi_t a, const cf_mpi_t b, const cf_mpi_t mod);
void    cf_mpi_mulm_no_alloc(cf_mpi_t *r, const cf_mpi_t a, const cf_mpi_t b, const cf_mpi_t mod);
void    cf_mpi_divexact(cf_mpi_t *r, const cf_mpi_t a, const cf_mpi_t b);
void    cf_mpi_sub(cf_mpi_t *r, const cf_mpi_t a, const cf_mpi_t b);
void    cf_mpi_sub_ui(cf_mpi_t *r, const cf_mpi_t a, unsigned long int b);
void    cf_mpi_subm(cf_mpi_t *r, const cf_mpi_t a, const cf_mpi_t b, const cf_mpi_t mod);
void    cf_mpi_mod(cf_mpi_t *r, const cf_mpi_t a, const cf_mpi_t m);
void    cf_mpi_set_ui(cf_mpi_t *r, ulint i);
int8_t  cf_mpi_inv(cf_mpi_t *rop, const cf_mpi_t op1, const cf_mpi_t op2);
void    cf_mpi_powm(cf_mpi_t *rop, const cf_mpi_t base, const cf_mpi_t exp, const cf_mpi_t mod);
void    cf_mpi_powm_no_alloc(cf_mpi_t *rop, const cf_mpi_t base, const cf_mpi_t exp, const cf_mpi_t mod);
void    cf_mpi_powm_ui(cf_mpi_t *rop, const cf_mpi_t base, ulint exp, const cf_mpi_t mod);
void    cf_mpi_powm_ui_no_alloc(cf_mpi_t *rop, const cf_mpi_t base, ulint exp, const cf_mpi_t mod);
void    cf_mpi_pow_ui(cf_mpi_t *rop, const cf_mpi_t base, ulint exp);

cf_errno  cf_mpi_import_char(cf_mpi_t *dest, const char *orig);
cf_errno  cf_mpi_import_uchar(cf_mpi_t *dest, const uchar *orig, size_t orig_len);
cf_errno  cf_mpi_export_char(char **dest, const cf_mpi_t a);
cf_errno  cf_mpi_export_uchar(uchar **dest, size_t *dest_len, const cf_mpi_t a);

void cf_mpi_import(cf_mpi_t *rop, size_t count, int order, size_t size, int endian,
                   size_t nails, const void *op);


////////////////////////////////////////////
// GMP stuff
////////////////////////////////////////////
//!< Get the number of bits of a given mpz_t.
#define MPZ_NBITS(x) (mpz_sizeinbase(x, 2))

//! @brief Creates the default GMP random generator state and set a random seed.
cf_errno cf_gmp_randstate_create(gmp_randstate_t *prng);

//! @brief Clear the GMP random state.
void cf_gmp_randstate_clear(gmp_randstate_t prng);

//! @brief Allocates and generates a random number of *exactly* \p n bits.
void cf_gmp_random_nbits(mpz_t x, mp_bitcnt_t n, gmp_randstate_t prng);

//! @brief Allocates and generates a prime of roughly \f$n\f$ bits.
cf_errno cf_gmp_random_prime(mpz_t p, mp_bitcnt_t n, gmp_randstate_t prng);
//! @brief Same as cf_gmp_random_prime, but p is already allocated (to be used in loops)
cf_errno cf_gmp_random_prime_no_alloc(mpz_t p, mp_bitcnt_t n, gmp_randstate_t prng);

//! @brief Allocates and generates a random number modulo \p mod
void cf_gmp_random_mod(mpz_t x, mpz_t mod, gmp_randstate_t prng);
void cf_gmp_random_mod_no_alloc(mpz_t x, mpz_t mod, gmp_randstate_t prng);

//! @brief Converts a previously allocated GMP to string.
cf_errno cf_gmp_to_char(char **dest, const mpz_t a);

//! @brief Converts a string to a newly allocated GMP.
cf_errno cf_char_to_gmp(mpz_t a, const char *s);

//! @brief Converts a GMP to a newly allocated uchar string.
cf_errno cf_gmp_to_uchar(uchar **dest, size_t *dest_len, const mpz_t a);

//! @brief Convert a previously allocated uchar string to a newly allocated GMP.
cf_errno cf_uchar_to_gmp(mpz_t a, const uchar *u, size_t u_len);

/**
 * Conversion between gcrypt and gmp
 */
cf_errno    gcrypt_to_mpz(mpz_t dest, const gcry_mpi_t a);
cf_errno    mpz_to_gcrypt(gcry_mpi_t *dest, const mpz_t a);

/**
 * Inits gcrypt
 */
cf_errno    gcrypt_init(void);

#ifdef __cplusplus
}
#endif
