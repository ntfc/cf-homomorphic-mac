#pragma once

#include "mpi-cf.h"

#ifdef __cplusplus
extern "C" {
#endif

//! @brief A message is simply a MPI number.
typedef cf_mpi_t cf_message_t;
//! @brief A label is simply a MPI number.
//! @todo How to make sure that a label is exactly lambda bits?
typedef cf_mpi_t cf_label_t;

// write in nwritten the number of bytes read
void          cf_message_free(cf_message_t msg);
void          cf_message_copy(cf_message_t *dest, const cf_message_t msg);
void          cf_messages_free(cf_message_t *msgs, size_t nmsgs);

// write in nwritten the number of bytes read
void          cf_label_free(cf_label_t lbl);
void          cf_labels_free(cf_label_t *lbls, size_t nlbls);

#ifdef __cplusplus
}
#endif
