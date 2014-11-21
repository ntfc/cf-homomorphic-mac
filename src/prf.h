#pragma once

#include <gcrypt.h>
#include "mpi-cf.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef gcry_cipher_hd_t prf_hd_t;

cf_errno    prf_encrypt_mpi(cf_mpi_t *dest, const cf_mpi_t k, const cf_mpi_t mod,
                            const cf_mpi_t a, prf_hd_t prf);

cf_errno    prf_create_handler(prf_hd_t *hd, const cf_mpi_t key, size_t byte_len);
void        prf_close_handler(prf_hd_t prf);

#ifdef __cplusplus
}
#endif
