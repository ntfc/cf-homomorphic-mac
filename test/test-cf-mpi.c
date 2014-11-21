#include "test.h"

#define _mpz_dump(a)  gmp_fprintf(stderr, "0x%Zx", a)

static const char TNAME[] = "cf-mpi";

static cf_errno cf_err = E_NOERR;

static void _test_cf_mpi_init() {
  cf_mpi_t a = NULL;
  mpz_t b;

  cf_mpi_init(&a);
  mpz_init(b);

  cf_err = mpz_cmp(a->mpi, b) == 0 ? E_NOERR : E_TEST_ERROR;

  cf_mpi_free(a);
  mpz_clear(b);
}
static void _test_cf_mpi_init2(uint nbits) {
  cf_mpi_t a = NULL;
  mpz_t b;

  cf_mpi_init2(&a, nbits);
  mpz_init2(b, nbits);

  cf_err = (mpz_cmp(a->mpi, b) == 0 && MPZ_NBITS(a->mpi) == MPZ_NBITS(b))
           ? E_NOERR : E_TEST_ERROR;

  cf_mpi_free(a);
  mpz_clear(b);
}
static void _test_cf_mpi_copy(uint nbits, cf_randstate_t prng) {
  cf_mpi_t a = NULL, b = NULL;

  cf_mpi_random_nbits(&a, nbits, prng);
  cf_mpi_copy(&b, a);
  cf_err = (mpz_cmp(a->mpi, b->mpi) == 0) ? E_NOERR : E_TEST_ERROR;

  cf_mpi_free(a);
  cf_mpi_free(b);
}

cf_errno test_cf_mpi(test_params_t *params) {
  cf_randstate_t prng = NULL;
  cf_mpi_library_init(&prng);
  _test_cf_mpi_init();
  if( cf_err != E_NOERR) {
    TEST_LOG_PRINT(TNAME, "cf_mpi_init fail\n");
  }
  _test_cf_mpi_init2(params->nbits);
  if( cf_err != E_NOERR) {
    TEST_LOG_PRINT(TNAME, "cf_mpi_init2 fail\n");
  }
  _test_cf_mpi_copy(params->nbits, prng);
  if( cf_err != E_NOERR) {
    TEST_LOG_PRINT(TNAME, "cf_mpi_copy fail\n");
  }
  cf_mpi_library_free(prng);
  return cf_err;
}
