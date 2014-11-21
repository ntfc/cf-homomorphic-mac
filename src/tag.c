#include <stdio.h>
#include "tag.h"
#include "support.h"

void cf_tag_new(cf_tag_t *tag, size_t degree) {
  (*tag) = malloc(sizeof(struct cf_tag_s));
  assert((*tag));
  (*tag)->n = degree + 1;
  (*tag)->y = calloc((*tag)->n, sizeof(cf_mpi_t));
}

void cf_tag_set_coeffs(cf_tag_t tag, cf_mpi_t *coeffs, size_t len) {
  size_t i;
  assert(len == tag->n);
  for(i = 0; i < len; i++) {
    //tag->y[i] = coeffs[i];
    cf_mpi_copy(&(tag->y[i]), coeffs[i]);
  }
}

cf_tag_t cf_tag_copy(const cf_tag_t tag) {
  size_t i = 0;
  cf_tag_t n = NULL;
  cf_tag_new(&n, CF_TAG_DEGREE(tag));
  for(i = 0; i < tag->n; i++) {
    //n->y[i] = gcry_mpi_copy(tag->y[i]);
    cf_mpi_copy(&(n->y[i]), tag->y[i]);
  }
  return n;
}

void cf_tag_free(cf_tag_t tag) {
  size_t i;
  if(tag != NULL) {
    if(tag->y != NULL) {
      for(i = 0; i < tag->n; i++) {
        if(tag->y[i] != NULL) {
          cf_mpi_free(tag->y[i]);
          tag->y[i] = NULL;
        }
        cf_mpi_free(tag->y[i]);
      }
      free(tag->y);
      tag->y = NULL;
    }
    free(tag);
    tag = NULL;
  }
}

void cf_tags_new(cf_tag_t **tags, size_t ntags) {
  *tags = calloc(ntags, sizeof(cf_tag_t));
}

void cf_tags_free(cf_tag_t *tags, size_t ntags) {
  size_t i;
  if(tags != NULL) {
    for(i = 0; i < ntags; i++)
      cf_tag_free(tags[i]);
    free(tags);
    tags = NULL;
  }
}
