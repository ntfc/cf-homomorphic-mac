#include "prf.h"

cf_errno prf_create_handler(prf_hd_t *hd, const cf_mpi_t key, size_t byte_len) {
  size_t nwritten = 0;
  uchar *key_vec = NULL;
  cf_errno cf_err = E_NOERR;
  

  // create the cipher handler
  gcry_err = gcry_cipher_open(hd, GCRY_CIPHER_AES, GCRY_CIPHER_MODE_ECB, GCRY_CIPHER_SECURE);
  if(IS_GCRY_ERROR) {
    return E_GCRY_ERROR;
  }

  // convert the mpi key to an uchar
  cf_err = cf_mpi_export_uchar(&key_vec, &nwritten, key);
  if(IS_GCRY_ERROR) {
    return E_GCRY_ERROR;
    //gcry_dump_error(err);
  }
  uchar key_vec_no_alloc[nwritten];
  memcpy(key_vec_no_alloc, key_vec, nwritten);
  free(key_vec); key_vec = NULL;
  if(nwritten != byte_len) {
    // TODO: move this to debug
    //printf("prf_create_handler: nwritten: %zu, sizeof(key): %zu, byte_len: %zu\n", nwritten, sizeof(key_vec), byte_len);
    assert(nwritten == byte_len && "nwritten should be the same as byte_len");
  }
  assert(nwritten > 0 && nwritten <= byte_len);
  // ..and set it as the cipher key
  gcry_err = gcry_cipher_setkey(*hd, key_vec_no_alloc, byte_len);
  if(IS_GCRY_ERROR) {
    if(key_vec != NULL) {
      free(key_vec);
    }
    prf_close_handler(*hd);
    *hd = NULL;
    
    return E_GCRY_ERROR;
  }
  
  return cf_err;
}

void prf_close_handler(prf_hd_t prf) {
  gcry_cipher_close(prf);
}

cf_errno prf_encrypt_mpi(cf_mpi_t *r, const cf_mpi_t key, const cf_mpi_t mod,
                         const cf_mpi_t a, prf_hd_t prf) {
  cf_errno cf_err;
  uint lambda;
  size_t byte_len;
  size_t nwritten = 0;
  uchar *r_vec = NULL;
  uchar *a_vec = NULL, *tmp = NULL;
  int close_prf = 0;
  
  if(!key) return E_NO_SK_KEY;
  if(!mod) return E_NO_DATA;
  if(!a) return E_NO_DATA;
  
  //lambda = gcry_mpi_get_nbits(sk->k);
  lambda = cf_mpi_nbits(key);
  if((lambda % 8) != 0) return E_INVAL_SECPARAM;
  byte_len =  lambda/8;
  
  // create the prf handler if necessary
  if(prf == NULL) {
    cf_err = prf_create_handler(&prf, key, byte_len);
    return cf_err;
    close_prf = 1;
  }
  // convert the mpi to an uchar
  cf_err = cf_mpi_export_uchar(&tmp, &nwritten, a);
  if(cf_err)
    return cf_err;

  if(nwritten != byte_len) {
    // pad the a_vec with zeros
    a_vec = calloc(byte_len, sizeof(uchar));
    memcpy(a_vec + (byte_len - nwritten), tmp, nwritten);
  }
  else {
    a_vec = tmp;
  }
  // number of written bytes in lbl_vec must be less or equal to the security parameter
  assert(nwritten > 0 && nwritten <= byte_len);
  // encrypt the mpi to r_vec
  r_vec = calloc(byte_len, sizeof(uchar));
  gcry_err = gcry_cipher_encrypt(prf, r_vec, byte_len, a_vec, byte_len);
  assert(gcry_err == 0);
  if(IS_GCRY_ERROR)
    return E_GCRY_ERROR;

  // convert the r_vec to mpi
  cf_err = cf_mpi_import_uchar(r, r_vec, byte_len);
  //gcry_err = uchar_to_mpi(r, &nwritten, r_vec, byte_len);
  if(cf_err)
    return cf_err; // TODO: cleanup
  
  // if r >= p, perform modular reduction
  // TODO: is this right?
  if(cf_mpi_cmp(*r, mod) >= 0)
    cf_mpi_mod(r, *r, mod);

  assert(cf_mpi_cmp(*r, mod) < 0);
  free(r_vec);
  if(tmp == a_vec) {
    free(tmp);
  }
  else {
    free(tmp);
    free(a_vec);
  }
  if(close_prf == 1) {
    prf_close_handler(prf);
  }
  return E_NOERR;
}
