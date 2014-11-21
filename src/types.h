/*
 * Various definitions of various CF types such as keys, tags or messages
 */

#pragma once
#include <assert.h>
#include <gcrypt.h>
#include <gmp.h>
#include <stdint.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief CF schemes.
 */
typedef enum cf_schemes {
  CF_1,     /**< Homomorphic MACs from One-Way Functions. */
  CF_2      /**< Compact Homomorphic MAC for circuits of bounded poly degree. */
} cf_scheme_e;

#define LINE_LEN_MIN 32 //!< Minimum number of bytes to read when parsing files.
#define CR 13           //!< Carriage return.
#define LF 10           //!< Line feed.

//! @brief The id of a graph edge is a size_t.
typedef size_t edge_id_t;
//! @brief The id of an arithmetic circuit's gate is and edge_id_t.
typedef edge_id_t gate_id_t;

// TODO: define all typedefs here, and the struct on the appropriate files
//! @brief Too lazy to type <tt>unsigned char</tt>.
typedef unsigned char uchar;
//! @brief Too lazy to type <tt> unsigned int</tt>.
typedef uint32_t uint;
//! @brief Too lazy to type <tt>unsigned long</tt>.
typedef unsigned long ulong;

typedef unsigned long int ulint;

#ifdef __cplusplus
}
#endif
