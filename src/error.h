/**
 * @file error.h
 * @todo Use argp_error() instead of cf_perror() in main.c
 */
#pragma once
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define E_NOERR            0   //!<  Everything OK.
#define E_NO_MEM           1   //!<  Out of memory.
#define E_NO_KEYPAIR       2   //!<  No key pair.
#define E_NO_EK_KEY        3   //!<  No evaluation key.
#define E_NO_SK_KEY        4   //!<  No secret key.
#define E_INVAL_EK_KEY     5   //!<  Invalid evaluation key.
#define E_INVAL_SK_KEY     6   //!<  Invalid secret key.
#define E_NO_MSG           7   //!<  No message.
#define E_NO_LBL           8   //!<  No label.
#define E_NO_TAG           9   //!<  No tag.
#define E_NO_ARITH        10   //!<  No arithmetic circuit.
#define E_NO_PRGM         11   //!<  No labeled program.
#define E_INVAL_MSG       12   //!<  Not a valid message.
#define E_INVAL_LBL       13   //!<  Not a valid label.
#define E_INVAL_TAG       14   //!<  Not a valid tag.
#define E_INVAL_ARITH     15   //!<  Not a valid arithmetic circuit.
#define E_MSG_SIZE        16   //!<  Not a valid message(s) size.
#define E_LBL_SIZE        17   //!<  Not a valid label(s) size.
#define E_TAG_SIZE        18   //!<  Not a valid tag(s) size
#define E_MSG_LBL_SIZE    19   //!<  Pair message/label must have the same length.
#define E_RANGE           20   //!<  Result out of range.
#define E_INVAL_NUM       21   //!<  Input number is not valid.
#define E_NO_DATA         22   //!<  Expected some data, but no data was found.
#define E_EK_FILE_FORMAT  23   //!<  Wrong evaluation key file format.
#define E_SK_FILE_FORMAT  24   //!<  Wrong secret key file format.
#define E_TAG_FILE_FORMAT 25   //!<  Wrong tag file format.
#define E_AC_FILE_FORMAT  26   //!<  Wrong arithmetic circuit file format.
#define E_FILE_FORMAT     27   //!<  Wrong file format.
#define E_INVAL_N_INPUTS  28   //!<  Wrong number of inputs.
#define E_GATE_ADD        29   //!<  Error adding gate to arithmetic circuit.
#define E_INVAL_GATE_TYPE 30   //!<  Invalid gate type.
#define E_GATE_INPUTS     31   //!<  Invalid number of gate inputs.
#define E_GATE_OUTPUT     32   //!<  Invalid number of gate ouputs.
#define E_INVAL_SECPARAM  33   //!<  Not a valid value for security parameter.
#define E_NO_ARITH_FILE   34   //!<  No arithmetic circuit file.
#define E_NO_DIR          35   //!<  No directory.
#define E_INVAL_DEGREE    36   //!<  Invalid degree bound.

#define E_VRFY            75   //!<  Verification ok.
#define E_NO_VRFY         76   //!<  Verification fail.

// error from external libraries
#define E_REGEX_ERROR     80   //!<  Regex error.
#define E_NTL_ERROR       81   //!<  Error code originated from NTL.
#define E_GMP_ERROR       82   //!<  Error code originated from GMP.
#define E_GCRY_ERROR      83   //!<  Error code originated from libgcrypt.
#define E_ERRNO_ERROR     84   //!<  Error code originated from errno.

#define E_TEST_ERROR      97   //!<  A test has failed.
#define E_NOT_IMPL        98   //!<  Function not yet implemented.
#define E_NOT_AVAIL       99   //!<  Function not yet available.
// TODO: tageval.c errors

//! Macro to check if a libgcrypt error has occurred.
#define IS_GCRY_ERROR     (gcry_err_code(gcry_err) != GPG_ERR_NO_ERROR)

//! Macro to check if a regex error has occurred.
#define IS_REGEX_ERROR    (regex_err != 0)

//! @brief An error code is simply an integer.
typedef int cf_errno;
//! @brief Human friendly representation of an error.
typedef struct cf_err_s cf_err_t;

//! Declaration of the global libgcrypt error variable.
extern gcry_error_t gcry_err;
//! Declaration of the global regex error variable.
extern int regex_err;


//! @brief Return a string describing the CF error.
const char *          str_cferror(cf_errno);
//! @brief Print a CF error message.
void                  cf_perror(const char *, cf_errno);

void                  gcrypt_perror(gcry_error_t);
void                  regex_perror(int);

#ifdef __cplusplus
}
#endif
