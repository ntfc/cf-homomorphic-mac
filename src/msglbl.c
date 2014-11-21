#include "msglbl.h"
#include "support.h"

void cf_message_free(cf_message_t msg) {
  cf_mpi_free(msg);
}

void cf_message_copy(cf_message_t *dest, const cf_message_t msg) {
  cf_mpi_copy(dest, msg);
}
void cf_messages_free(cf_message_t *msgs, size_t nmsgs) {
  size_t i;
  if(msgs != NULL) {
    for(i = 0; i < nmsgs; i++) {
      cf_message_free(msgs[i]);
    }
    free(msgs);
    msgs = NULL;
  }
}

void cf_label_free(cf_label_t lbl) {
  cf_mpi_free(lbl);
}

void cf_labels_free(cf_label_t *lbls, size_t nlbls) {
  size_t i;
  if(lbls != NULL) {
    for(i = 0; i < nlbls; i++) {
      cf_label_free(lbls[i]);
    }
    free(lbls);
    lbls = NULL;
  }
}

