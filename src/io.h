/**
 * @brief I/O
 * 
 * \c *_dump functions are useful for logging and debugging purposes.
 * 
 * \c _*write functions are used to write information to an output \c FILE.
 */
#pragma once
#include "keys.h"
#include "msglbl.h"
#include "tag.h"
#include "support.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LINE_LEN 512

#define cf_message_write  cf_mpi_write
#define cf_label_write    cf_mpi_write
#define cf_messages_write cf_mpis_write
#define cf_labels_write   cf_mpis_write
#define cf_message_read   cf_mpi_read
#define cf_label_read     cf_mpi_read
#define cf_messages_read  cf_mpis_read
#define cf_labels_read    cf_mpis_read
#define cf_message_dump   cf_mpi_dump
#define cf_label_dump     cf_mpi_dump
#define cf_messages_dump  cf_mpis_dump
#define cf_labels_dump    cf_mpis_dump

//! @brief Allocates and read a MPI from a string.
cf_errno cf_mpi_read(cf_mpi_t *mpi, const char *s);
//! @brief Allocates and reads an array of MPIs from a filename.
cf_errno cf_mpis_read(cf_mpi_t **mpis, size_t *n_mpis, const char *fn);

//! @brief Write a MPI to a FILE.
void cf_mpi_write(FILE *fp, const cf_mpi_t mpi);

//! @brief Write an array of MPIs to a FILE.
void cf_mpis_write(FILE *fp, const cf_mpi_t *mpis, size_t n);

//! @brief Write an array of MPIs to a FILE.
void cf_mpis_write(FILE *fp, const cf_mpi_t *mpis, size_t n);

//! @brief Dumps a MPI into \c stderr.
void cf_mpi_dump(const cf_mpi_t mpi);

//! @brief Dumps an array of MPIs into \c stderr.
void cf_mpis_dump(const cf_mpi_t *mpis, size_t n_mpis);

void cf_tag_dump(const cf_tag_t tag);
void cf_tags_dump(const cf_tag_t *tags, size_t n_tags);

void cf_keys_dump(const cf_keys_t keys);
void cf_key_sk_dump(const cf_key_sk_t sk);
void cf_key_ek_dump(const cf_key_ek_t ek);

// if fn = NULL, write to stdout
cf_errno    cf_keys_write(const char *fn, const cf_keys_t keys);

// if fn = NULL, write to stdout
cf_errno    cf_key_sk_write(FILE *fp, const cf_key_sk_t sk);
cf_errno    cf_key_sk_read(cf_key_sk_t *key, const char *fn);

cf_errno    cf_key_ek_write(FILE *fp, const cf_key_ek_t ek);
cf_errno    cf_key_ek_read(cf_key_ek_t *key, const char *fn);

// tags
cf_errno    cf_tag_write(const char *fn, const cf_tag_t tag);

cf_errno    cf_tags_read(cf_tag_t **tags, size_t *n_tags, const char *fn);
cf_errno    cf_tags_write(const char *fn, const cf_tag_t *tags, size_t taglen);

// file exists for reading?
int         file_exists(const char *fn);

#ifdef __cplusplus
}
#endif
