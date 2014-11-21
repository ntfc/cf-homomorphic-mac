#pragma once
//#include "mpi-cf.h"
#include "msglbl.h"
#include "tag.h"

#ifdef __cplusplus
extern "C" {
#endif

void      cf_message_random(cf_message_t *msg, cf_mpi_t mod, cf_randstate_t prng);
void      cf_label_random(cf_label_t *lbl, uint nbits, cf_randstate_t prng);
void      cf_messages_random(cf_message_t **msgs, size_t n_msgs, cf_mpi_t mod, cf_randstate_t prng);
void      cf_labels_random(cf_label_t **lbls, size_t n_lbls, uint nbits, cf_randstate_t prng);

void      cf_tag_random(cf_tag_t *tag, size_t degree, cf_mpi_t mod, cf_randstate_t prng);  


#ifdef __cplusplus
}
#endif
