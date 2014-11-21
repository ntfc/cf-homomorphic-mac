#include <fcntl.h> // open
#include <stdio.h>
#include <sys/stat.h> // open
#include <unistd.h> // read and close
#include "mpi-cf.h"

cf_errno cf_mpi_library_init(cf_randstate_t *prng) {
  cf_errno cf_err = E_NOERR;
  *prng = calloc(1, sizeof(struct cf_randstate_s));
  cf_err = cf_gmp_randstate_create(&((*prng)->state));
  if(cf_err == E_NOERR) {
    (*prng)->_alloced = 1;
    return cf_err;
  }
  // in case of error undo everything
  free(*prng);
  *prng = NULL;
  
  return cf_err;
}

void cf_mpi_library_free(cf_randstate_t prng) {
  if(prng != NULL) {
    if(prng->_alloced == 1) {
      cf_gmp_randstate_clear(prng->state);
      prng->_alloced = 0;
    }
    free(prng);
  }
}

static inline void cf_mpi_new(cf_mpi_t *a) {
  *a = calloc(1, sizeof(struct cf_mpi_s));
  (*a)->_alloced = 0;
}

void cf_mpi_init(cf_mpi_t *a) {
  cf_mpi_init2(a, 1);
}
void cf_mpi_init2(cf_mpi_t *a, uint nbits) {
  *a = calloc(1, sizeof(struct cf_mpi_s));
  mpz_init2((*a)->mpi, nbits);
  (*a)->_alloced = 1;
}
void cf_mpi_free(cf_mpi_t a) {
  if(a != NULL) {
    if(a->_alloced > 0) {
      mpz_clear(a->mpi);
    }
    a->_alloced = 0;
    free(a); a = NULL;
  }
}
void cf_mpi_copy(cf_mpi_t *dest, const cf_mpi_t orig) {
  if(orig != NULL) {
    cf_mpi_init2(dest, cf_mpi_nbits(orig));
    mpz_set((*dest)->mpi, orig->mpi);
    (*dest)->_alloced = 1;
  }
}

cf_errno cf_mpi_random_prime(cf_mpi_t *prime, uint nbits, cf_randstate_t prng) {
  cf_errno cf_err = E_NOERR;
  *prime = calloc(1, sizeof(struct cf_mpi_s));
  cf_err = cf_gmp_random_prime((*prime)->mpi, nbits, prng->state);
  if(cf_err != E_NOERR) {
    free(*prime); *prime = NULL;
  }
  (*prime)->_alloced = 1;
  return cf_err;
}

cf_errno cf_mpi_random_prime_no_alloc(cf_mpi_t *prime, uint nbits, cf_randstate_t prng) {
  cf_errno cf_err = E_NOERR;
  assert((*prime) != NULL && (*prime)->_alloced == 1);
  cf_err = cf_gmp_random_prime_no_alloc((*prime)->mpi, nbits, prng->state);
  if(cf_err != E_NOERR) {
    //free(*prime); *prime = NULL;
    assert(cf_err == E_NOERR);
  }
  (*prime)->_alloced = 1;
  return cf_err;
}

void cf_mpi_random_nbits(cf_mpi_t *r, uint nbits, cf_randstate_t prng) {
  *r = calloc(1, sizeof(struct cf_mpi_s));
  cf_gmp_random_nbits((*r)->mpi, nbits, prng->state);
  (*r)->_alloced = 1;
}

void cf_mpi_random_mod(cf_mpi_t *r, const cf_mpi_t mod, cf_randstate_t prng) {
  *r = calloc(1, sizeof(struct cf_mpi_s));
  cf_gmp_random_mod((*r)->mpi, mod->mpi, prng->state);
  (*r)->_alloced = 1;
}
void cf_mpi_random_mod_no_alloc(cf_mpi_t *r, cf_mpi_t mod, cf_randstate_t prng) {
  assert((*r) != NULL && (*r)->_alloced == 1);
  cf_gmp_random_mod_no_alloc((*r)->mpi, mod->mpi, prng->state);
  (*r)->_alloced = 1;
}

int cf_mpi_probab_prime(const cf_mpi_t p) {
  if(p == NULL) return -1;
  return mpz_probab_prime_p(p->mpi, 25); // 25 reps is a reasonable number
}

void cf_mpi_add(cf_mpi_t *r, const cf_mpi_t a, const cf_mpi_t b) {
  assert(a && b);
  if((*r) == NULL) // in case of an already allocated rop
    cf_mpi_new(r);
  mpz_add((*r)->mpi, a->mpi, b->mpi);
  (*r)->_alloced = 1;
}
void cf_mpi_add_ui(cf_mpi_t *r, const cf_mpi_t a, unsigned long int b) {
  assert(a);
  if((*r) == NULL) // in case of an already allocated rop
    cf_mpi_new(r);
  mpz_add_ui((*r)->mpi, a->mpi, b);
  (*r)->_alloced = 1;
}
void cf_mpi_addm(cf_mpi_t *r, const cf_mpi_t a, const cf_mpi_t b, const cf_mpi_t mod) {
  assert(a && b);
  cf_mpi_add(r, a, b);
  if(mod != NULL) {
    mpz_mod((*r)->mpi, (*r)->mpi, mod->mpi);
  }
}

void cf_mpi_mul(cf_mpi_t *r, const cf_mpi_t a, const cf_mpi_t b) {
  assert(a && b);
  if((*r) == NULL) {// in case of an already allocated rop
    cf_mpi_new(r);
  }
  mpz_mul((*r)->mpi, a->mpi, b->mpi);
  (*r)->_alloced = 1;
}
void cf_mpi_mul_ui(cf_mpi_t *r, const cf_mpi_t a, unsigned long int b) {
  assert(a);
  if((*r) == NULL) {// in case of an already allocated rop
    cf_mpi_new(r);
  }
  mpz_mul_ui((*r)->mpi, a->mpi, b);
  (*r)->_alloced = 1;
}
void cf_mpi_mulm(cf_mpi_t *r, const cf_mpi_t a, const cf_mpi_t b, const cf_mpi_t mod) {
  assert(a && b);
  cf_mpi_mul(r, a, b);
  if(mod != NULL) {
    mpz_mod((*r)->mpi, (*r)->mpi, mod->mpi);
  }
}

void cf_mpi_mulm_no_alloc(cf_mpi_t *r, const cf_mpi_t a, const cf_mpi_t b, const cf_mpi_t mod) {
  assert((*r) != NULL);
  assert(a && b);
  mpz_mul((*r)->mpi, a->mpi, b->mpi);
  if(mod != NULL) {
    mpz_mod((*r)->mpi, (*r)->mpi, mod->mpi);
  }
  (*r)->_alloced = 1;
}

void cf_mpi_divexact(cf_mpi_t *r, const cf_mpi_t a, const cf_mpi_t b) {
  assert(a && b);
  if((*r) == NULL) {// in case of an already allocated rop
    cf_mpi_new(r);
  }
  mpz_divexact((*r)->mpi, a->mpi, b->mpi);
  (*r)->_alloced = 1;
}

void cf_mpi_sub(cf_mpi_t *r, const cf_mpi_t a, const cf_mpi_t b) {
  assert(a && b);
  if((*r) == NULL) // in case of an already allocated rop
    cf_mpi_new(r);
  mpz_sub((*r)->mpi, a->mpi, b->mpi);
  (*r)->_alloced = 1;
}
void cf_mpi_sub_ui(cf_mpi_t *r, const cf_mpi_t a, unsigned long int b) {
  assert(a);
  if((*r) == NULL) // in case of an already allocated rop
    cf_mpi_new(r);
  mpz_sub_ui((*r)->mpi, a->mpi, b);
  (*r)->_alloced = 1;
}
void cf_mpi_subm(cf_mpi_t *r, const cf_mpi_t a, const cf_mpi_t b, const cf_mpi_t mod) {
  assert(a && b);
  cf_mpi_sub(r, a, b);
  if(mod != NULL) {
    mpz_mod((*r)->mpi, (*r)->mpi, mod->mpi);
  }
}

void cf_mpi_mod(cf_mpi_t *r, const cf_mpi_t a, const cf_mpi_t mod) {
  assert(a && mod);
  if((*r) == NULL) // in case of an already allocated rop
    cf_mpi_new(r);
  mpz_mod((*r)->mpi, a->mpi, mod->mpi);
  (*r)->_alloced = 1;
}

void cf_mpi_set_ui(cf_mpi_t *r, ulint i) {
  cf_mpi_init(r);
  mpz_set_ui((*r)->mpi, i);
  (*r)->_alloced = 1;
}

int8_t cf_mpi_inv(cf_mpi_t *rop, const cf_mpi_t op1, const cf_mpi_t op2) {
  assert(cf_mpi_cmp_ui(op2, 0x0));
  int8_t ret = 0;
  if((*rop) == NULL) // in case of an already allocated rop
    cf_mpi_new(rop);
  ret = mpz_invert((*rop)->mpi, op1->mpi, op2->mpi);
  if(ret == 0) { // inverse does not exist
    free(*rop);
    *rop = NULL;
  }
  else
    (*rop)->_alloced = 1;
  return ret;
}

void cf_mpi_powm(cf_mpi_t *rop, const cf_mpi_t base, const cf_mpi_t exp, const cf_mpi_t mod) {
  if((*rop) == NULL) // in case of an already allocated rop
    cf_mpi_new(rop);
  mpz_powm((*rop)->mpi, base->mpi, exp->mpi, mod->mpi);
  (*rop)->_alloced = 1;
}
void cf_mpi_powm_no_alloc(cf_mpi_t *rop, const cf_mpi_t base, const cf_mpi_t exp, const cf_mpi_t mod) {
  assert((*rop) != NULL);
  mpz_powm((*rop)->mpi, base->mpi, exp->mpi, mod->mpi);
  (*rop)->_alloced = 1;
}

void cf_mpi_powm_ui(cf_mpi_t *rop, const cf_mpi_t base, ulint exp, const cf_mpi_t mod) {
  if((*rop) == NULL) // in case of an already allocated rop
    cf_mpi_new(rop);
  assert((*rop)->mpi != NULL);
  mpz_powm_ui((*rop)->mpi, base->mpi, exp, mod->mpi);
  (*rop)->_alloced = 1;
}

void cf_mpi_powm_ui_no_alloc(cf_mpi_t *rop, const cf_mpi_t base, ulint exp, const cf_mpi_t mod) {
  assert((*rop) != NULL);
  mpz_powm_ui((*rop)->mpi, base->mpi, exp, mod->mpi);
  (*rop)->_alloced = 1;
}

void cf_mpi_pow_ui(cf_mpi_t *rop, const cf_mpi_t base, ulint exp) {
  if((*rop) == NULL) // in case of an already allocated rop
    cf_mpi_new(rop);
  mpz_pow_ui((*rop)->mpi, base->mpi, exp);
  (*rop)->_alloced = 1;
}

cf_errno  cf_mpi_import_char(cf_mpi_t *dest, const char *orig) {
  cf_errno cf_err = E_NOERR;
  *dest = calloc(1, sizeof(struct cf_mpi_s));
  if( (cf_err = cf_char_to_gmp((*dest)->mpi, orig)) == E_NOERR)
    (*dest)->_alloced = 1;
  return cf_err;
}

cf_errno  cf_mpi_import_uchar(cf_mpi_t *dest, const uchar *orig, size_t orig_len) {
  cf_errno cf_err = E_NOERR;

  *dest = calloc(1, sizeof(struct cf_mpi_s));
  if( (cf_err = cf_uchar_to_gmp((*dest)->mpi, orig, orig_len)) == E_NOERR)
    (*dest)->_alloced = 1;
  return cf_err;
}

cf_errno  cf_mpi_export_char(char **dest, const cf_mpi_t a) {
  cf_errno cf_err = E_NOERR;
  if(a != NULL) {
    cf_err = cf_gmp_to_char(dest, a->mpi);
  }
  return cf_err;
}

cf_errno  cf_mpi_export_uchar(uchar **dest, size_t *dest_len, const cf_mpi_t a) {
  cf_errno cf_err = E_NOERR;
  if(a != NULL) {
    cf_err = cf_gmp_to_uchar(dest, dest_len, a->mpi);
  }
  return cf_err;
}

void cf_mpi_import(cf_mpi_t *rop, size_t count, int order, size_t size, int endian,
                   size_t nails, const void *op) {
  cf_mpi_new(rop);
  mpz_import((*rop)->mpi, count, order, size, endian, nails, op);
  (*rop)->_alloced = 1;
}

/**
 * @brief Creates the default GMP random generator state and set a random seed.
 * 
 * First, a seed of 128 bits is generated from /dev/random. Then the state is 
 * created and properly set.
 * 
 * @param[out] prng GMP state
 * @returns E_ERRNO_ERROR If there is an error with /dev/random.
 * @returns E_NOERR If everything is OK.
 */
cf_errno cf_gmp_randstate_create(gmp_randstate_t *prng) {
  int dev_random_fd = -1, n_limbs, i;
  mpz_t seed;
  mp_limb_t tmp = 0x0;
  // use a 128bit seed by default
  dev_random_fd = open("/dev/random", O_RDONLY);
  if(dev_random_fd == -1) {
    return E_ERRNO_ERROR;
  }
  #if GMP_LIMB_BITS==64 // read 2 limbs from system entropy pool
    n_limbs = 2;
  #else // GMP_LIMB_BITS==32 // read 4 limbs from system entropy pool
    n_limbs = 4;
  #endif
  mp_limb_t limbs[n_limbs];
  for(i = 0; i < n_limbs; i++) {
    if(read(dev_random_fd, &(tmp) , GMP_LIMB_BITS/8) == -1) {
      close(dev_random_fd);
      return E_ERRNO_ERROR;
    }
    limbs[i] = tmp;
  }
  close(dev_random_fd);
  gmp_randinit_default(*prng);
  
  //gmp_printf("seed: %Nx\n", limbs);
  gmp_randseed(*prng, mpz_roinit_n(seed, limbs, n_limbs));
  return E_NOERR;
}

void cf_gmp_randstate_clear(gmp_randstate_t prng) {
  gmp_randclear(prng);
}

void cf_gmp_random_nbits(mpz_t x, mp_bitcnt_t n, gmp_randstate_t prng) {
  mpz_init2(x, n);
  do {
    mpz_urandomb(x, prng, n);
  } while(MPZ_NBITS(x) != n);
}

//! @returns E_NOERR Everything is OK.
cf_errno cf_gmp_random_prime(mpz_t p, mp_bitcnt_t n, gmp_randstate_t prng) {
  mpz_init2(p, n);
  mpz_urandomb(p, prng, n);
  
  mpz_nextprime(p, p);
  return E_NOERR;
}
// p is already alloced
cf_errno cf_gmp_random_prime_no_alloc(mpz_t p, mp_bitcnt_t n, gmp_randstate_t prng) {
  mpz_urandomb(p, prng, n);
  
  mpz_nextprime(p, p);
  return E_NOERR;
}

void cf_gmp_random_mod(mpz_t x, mpz_t mod, gmp_randstate_t prng) {
  mpz_init2(x, MPZ_NBITS(mod));
  mpz_urandomm(x, prng, mod);
}

void cf_gmp_random_mod_no_alloc(mpz_t x, mpz_t mod, gmp_randstate_t prng) {
  mpz_urandomm(x, prng, mod);
}

//! @returns E_NOERR Everything is OK.
cf_errno cf_gmp_to_char(char **dest, const mpz_t a) {
  *dest = mpz_get_str(NULL, 16, a);
  return E_NOERR;
}

//! @returns E_NOERR Everything is OK.
cf_errno cf_char_to_gmp(mpz_t a, const char *s) {
  int base = 16;
  if(strlen(s) > 1 && s[0] == '0') {
    switch(s[1]) {
      case 'b': base = 0; break;
      case 'B': base = 0; break;
      case 'x': base = 0; break;
      case 'X': base = 0; break;
    }    
  }
  mpz_init(a);
  if(mpz_set_str(a, s, base) == -1) {
    mpz_clear(a);
    return E_INVAL_NUM;
  }
  return E_NOERR;
}

//! @returns E_NOERR Everything is OK.
cf_errno cf_gmp_to_uchar(uchar **dest, size_t *dest_len, const mpz_t a) {
  size_t n_words = 0;
  // from mpz_export: how to get the necessary space for buffer buf
  size_t nails = 0;
  size_t numb = 8*sizeof(uchar) - nails;
  (*dest_len) = (mpz_sizeinbase(a, 2) + numb-1) / numb;
  
  (*dest) = calloc((*dest_len), sizeof(uchar));
  mpz_export(*dest, &n_words, 1, sizeof(uchar), 1, nails, a);
  assert(n_words == (*dest_len));
  return E_NOERR;
}

//! @returns E_NOERR Everything is OK.
cf_errno cf_uchar_to_gmp(mpz_t a, const uchar *u, size_t u_len) {
  mpz_init2(a, u_len * 8);
  mpz_import(a, u_len, 1, sizeof(uchar), 1, 0, u);
  return E_NOERR;
}

cf_errno gcrypt_to_mpz(mpz_t dest, const gcry_mpi_t a) {
  cf_errno cf_err = E_NOERR;
  uchar *buf = NULL;
  size_t n_bytes = 0;
  int MSB_FIRST = 1;
  gcry_err = gcry_mpi_aprint(GCRYMPI_FMT_USG, &buf, &n_bytes, a);
  if(IS_GCRY_ERROR)
    cf_err = E_GCRY_ERROR;
  if(cf_err == E_NOERR) {
    mpz_init(dest);
    mpz_import(dest, n_bytes, MSB_FIRST, sizeof(buf[0]), MSB_FIRST, 0, buf);
    if(buf != NULL) {
      free(buf);
    }
  }
  return cf_err;
}

cf_errno mpz_to_gcrypt(gcry_mpi_t *dest, const mpz_t a) {
  cf_errno cf_err = E_NOERR;
  int MSB_FIRST = 1;
  size_t n_words = 0, n_scanned = 0;
  // from mpz_export: how to get the necessary space for buffer buf
  size_t nails = 0;
  size_t numb = 8*sizeof(uchar) - nails;
  size_t count = (mpz_sizeinbase (a, 2) + numb-1) / numb;
  uchar buf[count];
  
  mpz_export(buf, &n_words, MSB_FIRST, sizeof(uchar), MSB_FIRST, nails, a);
  
  gcry_err = gcry_mpi_scan(dest, GCRYMPI_FMT_USG, buf, n_words, &n_scanned);
  if(IS_GCRY_ERROR)
    cf_err = E_GCRY_ERROR;  
  // FIXME: check not needed?
  //assert(n_scanned == count);
  
  return cf_err;
}

cf_errno gcrypt_init(void) {
  // is it already initialized?
  if( !gcry_control(GCRYCTL_INITIALIZATION_FINISHED_P) ) {
    /* Version check should be the very first call because it
     * makes sure that important subsystems are initialized. */
    if (!gcry_check_version (GCRYPT_VERSION)) {
      fprintf(stderr, "FATAL ERROR: libgcrypt version mismatch\n");
      exit(2);
    }
    /* We don't want to see any warnings, e.g. because we have not yet
     * parsed program options which might be used to suppress such
     * warnings */
    gcry_err = gcry_control (GCRYCTL_SUSPEND_SECMEM_WARN);
    if(IS_GCRY_ERROR)
      return E_GCRY_ERROR;
    /* ... If required, other initialization goes here.  Note that the
     * process might still be running with increased privileges and that
     * the secure memory has not been initialized. */

    /* Allocate a pool of 2k secure memory.  This make the secure memory
     * available and also drops privileges where needed. */
    gcry_err = gcry_control (GCRYCTL_INIT_SECMEM, 2048, 0);
    if(IS_GCRY_ERROR)
      return E_GCRY_ERROR;

    /* It is now okay to let Libgcrypt complain when there was/is
     * a problem with the secure memory. */
    gcry_err = gcry_control (GCRYCTL_RESUME_SECMEM_WARN);
    if(IS_GCRY_ERROR)
      return E_GCRY_ERROR;

    /* ... If required, other initialization goes here.  */
    
    /* Tell Libgcrypt that initialization has completed. */
    gcry_err = gcry_control (GCRYCTL_INITIALIZATION_FINISHED, 0);
    if(IS_GCRY_ERROR)
      return E_GCRY_ERROR;
  }
  return E_NOERR;
}
