#include "cf-random.h"

void cf_message_random(cf_message_t *msg, cf_mpi_t mod, cf_randstate_t prng) {
  cf_mpi_random_mod(msg, mod, prng);
}

void cf_label_random(cf_label_t *lbl, uint nbits, cf_randstate_t prng) {
  cf_mpi_random_nbits(lbl, nbits, prng);
  assert(cf_mpi_nbits(*lbl) == nbits);
}

void cf_messages_random(cf_message_t **msgs, size_t n_msgs, cf_mpi_t mod, cf_randstate_t prng) {
  size_t i;
  if(n_msgs > 0) {
    *msgs = calloc(n_msgs, sizeof(cf_message_t));
    for(i = 0; i < n_msgs; i++) {
      cf_message_random(&((*msgs)[i]), mod, prng);
    }
  }
}

void cf_labels_random(cf_label_t **lbls, size_t n_lbls, uint nbits, cf_randstate_t prng) {
  size_t i;
  if(n_lbls > 0) {
    *lbls = calloc(n_lbls, sizeof(cf_message_t));
    for(i = 0; i < n_lbls; i++) {
      cf_label_random(&((*lbls)[i]), nbits, prng);
    }
  }
}

void cf_tag_random(cf_tag_t *tag, size_t degree, cf_mpi_t mod, cf_randstate_t prng) {
  size_t i;
  cf_tag_new(tag, degree);

  for(i = 0; i < (*tag)->n; i++) {
    cf_mpi_random_mod(&((*tag)->y[i]), mod, prng);
  }
}
