#pragma once

#include "mpi-cf.h"

#ifdef __cplusplus
extern "C" {
#endif

#define       CF_TAG_DEGREE(t) ((t)->n - (size_t)1)

/**
 * @brief Structure to hold a tag used in the CF protocol.
 * 
 * The degree of tag is the number of elements in #y - 1.
 */
struct cf_tag_s {
  //! @brief Vector of #n tags.
  cf_mpi_t *y; 
  //! @brief Length of vector #y.
  size_t n;
};
//! @brief Type of a CF tag.
typedef struct cf_tag_s *cf_tag_t;

// allocs a vector of size degree + 1
void          cf_tag_new(cf_tag_t *, size_t degree);

void cf_tag_set_coeffs(cf_tag_t tag, cf_mpi_t *coeffs, size_t len);

void cf_tag_dump(const cf_tag_t tag);

cf_tag_t      cf_tag_copy(const cf_tag_t tag);

void          cf_tag_free(cf_tag_t tag);
void          cf_tags_new(cf_tag_t **tags, size_t ntags);
void          cf_tags_free(cf_tag_t *tags, size_t ntags);

#ifdef __cplusplus
}
#endif
