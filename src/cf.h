#pragma once
#include "arith.h"
#include "prf.h"
#include "support.h"
#include "keys.h"
#include "msglbl.h"
#include "tag.h"
#include "lblprgm.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * CF protocol
 *****************************************************************************/

cf_errno      cf_auth(cf_tag_t *, cf_key_sk_t, cf_label_t, cf_message_t, prf_hd_t);

cf_errno      cf_eval(cf_tag_t *, cf_key_ek_t, const arith_circ_t,
                      const cf_tag_t *, size_t, cf_tag_t *, cf_scheme_e);
cf_errno      cf_vrfy(cf_key_sk_t, cf_message_t, cf_program_t, cf_tag_t,
                      prf_hd_t);
//! @brief Compress a tag [y0, .., yd] into a A in G
cf_errno      _cf_compress_tag(cf_tag_t *, cf_key_ek_t, cf_tag_t);
cf_errno      _cf_eval_tags(cf_tag_t *result, const arith_circ_t a,  const cf_tag_t *tags,
                            size_t n_tags, const cf_mpi_t mod, cf_scheme_e);

cf_errno      cf_prf_encrypt_label(cf_mpi_t *r, const cf_key_sk_t sk, cf_label_t lbl,
                                   prf_hd_t prf);

#ifdef __cplusplus
}
#endif
