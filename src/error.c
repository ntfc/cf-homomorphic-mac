#include <errno.h>
#include <stdio.h>
#include "error.h"

#define MAX_CF_ERROR_LEN  100
gcry_error_t gcry_err  = NULL;
int          regex_err = 0;

/**
 * @brief Human friendly representation of an error.
 */
struct cf_err_s {
  cf_errno err_no;   /**< Error code. */
  char    *errname;  /**< Error code name. */
  char    *errmsg;   /**< Message describing the error. */
};

static cf_err_t CF_ERR[] = {
  { E_NOERR,           "E_NOERR",            "Everything OK"                                  },
  { E_NO_MEM,          "E_NO_MEM",           "Out of memory"                                  },
  { E_NO_KEYPAIR,      "E_NO_KEYPAIR",       "No key pair"                                    },
  { E_NO_EK_KEY,       "E_NO_EK_KEY",        "No evaluation key"                              },
  { E_NO_SK_KEY,       "E_NO_SK_KEY",        "No secret key"                                  },
  { E_INVAL_EK_KEY,    "E_INVAL_EK_KEY",     "Invalid evaluation key"                         },
  { E_INVAL_SK_KEY,    "E_INVAL_SK_KEY",     "Invalid secret key"                             },
  { E_NO_MSG,          "E_NO_MSG",           "No message"                                     },
  { E_NO_LBL,          "E_NO_LBL",           "No label"                                       },
  { E_NO_TAG,          "E_NO_TAG",           "No tag"                                         },
  { E_NO_ARITH,        "E_NO_ARITH",         "No arithmetic circuit"                          },
  { E_NO_PRGM,         "E_NO_PRGM",          "No labeled program"                             },
  { E_INVAL_MSG,       "E_INVAL_MSG",        "Not a valid message"                            },
  { E_INVAL_LBL,       "E_INVAL_LBL",        "Not a valid label"                              },
  { E_INVAL_TAG,       "E_INVAL_TAG",        "Not a valid tag"                                },
  { E_INVAL_ARITH,     "E_INVAL_ARITH",      "Not a valid arithmetic circuit"                 },
  { E_MSG_SIZE,        "E_MSG_SIZE",         "Not a valid message(s) size"                    },
  { E_LBL_SIZE,        "E_LBL_SIZE",         "Not a valid label(s) size"                      },
  { E_TAG_SIZE,        "E_TAG_SIZE",         "Not a valid tag(s) size"                        },
  { E_MSG_LBL_SIZE,    "E_MSG_LBL_SIZE",     "Pair message/label must have the same length"   },
  { E_RANGE,           "E_RANGE",            "Result out of range"                            },
  { E_INVAL_NUM,       "E_INVAL_NUM",        "Input number is not valid"                      },
  { E_NO_DATA,         "E_NO_DATA",          "Expected some data, but no data was found"      },
  { E_EK_FILE_FORMAT,  "E_EK_FILE_FORMAT",   "Wrong evaluation key file format"               },
  { E_SK_FILE_FORMAT,  "E_SK_FILE_FORMAT",   "Wrong secret key file format"                   },
  { E_TAG_FILE_FORMAT, "E_TAG_FILE_FORMAT",  "Wrong tag file format"                          },
  { E_AC_FILE_FORMAT,  "E_AC_FILE_FORMAT",   "Wrong arithmetic circuit file format"           },
  { E_FILE_FORMAT,     "E_FILE_FORMAT",      "Wrong file format"                              },
  { E_INVAL_N_INPUTS,  "E_INVAL_N_INPUTS",   "Wrong number of inputs"                         },
  { E_GATE_ADD,        "E_GATE_ADD",         "Error adding gate to arithmetic circuit"        },
  { E_INVAL_GATE_TYPE, "E_INVAL_GATE_TYPE",  "Invalid gate type"                              },
  { E_GATE_INPUTS,     "E_GATE_INPUTS",      "Invalid number of gate inputs"                  },
  { E_GATE_OUTPUT,     "E_GATE_OUTPUT",      "Invalid number of gate outputs"                 },
  { E_INVAL_SECPARAM,  "E_INVAL_SECPARAM",   "Not a valid value for security parameter"       },
  { E_NO_ARITH_FILE,   "E_NO_ARITH_FILE",    "No arithmetic circuit file"                     },
  { E_NO_DIR,          "E_NO_DIR",           "No directory"                                   },
  { E_INVAL_DEGREE,    "E_INVAL_DEGREE",     "Invalid degree bound"                           },
  { E_VRFY,            "E_VRFY",             "Verification ok"                                },
  { E_NO_VRFY,         "E_NO_VRFY",          "Verification fail"                              },
  { E_REGEX_ERROR,     "E_REGEX_ERROR",      "Regex error"                                    },
  { E_NTL_ERROR,       "E_NTL_ERROR",        "Error code originated from NTL"                 },
  { E_GMP_ERROR,       "E_GMP_ERROR",        "Error code originated from GMP"                 },
  { E_GCRY_ERROR,      "E_GCRY_ERROR",       "Error code originated from libgcrypt"           },
  { E_ERRNO_ERROR,     "E_ERRNO_ERROR",      "Error code originated from errno"               },
  { E_TEST_ERROR,      "E_TEST_ERROR",       "A test has failed"                              },
  { E_NOT_IMPL,        "E_NOT_IMPL",         "Function not yet implemented"                   },
  { E_NOT_AVAIL,       "E_NOT_AVAIL",        "Function not yet available"                     },
};

/**
 * Returns a pointer to string that describes the error code passed in the argument
 * \p cf_err_no.
 * 
 * @param[in] cf_err_no The CF error code.
 * 
 * @return A pointer to a string describing the error message, or \c NULL if the
 * error code is not found.
 */
const char *str_cferror(cf_errno cf_err_no) {
  int i;
  int CF_ERR_LEN;
  const char *s = NULL;
  char buf[MAX_CF_ERROR_LEN];
  
  switch(cf_err_no) {
    case E_ERRNO_ERROR: // errno.h
      s = strerror(errno);
      break;
    case E_GCRY_ERROR: // gcrypt.h  
      s = gcry_strerror(gcry_err);
      break;
    case E_GMP_ERROR: // gmp.h
      s = "GMP ERROR";
      break;
    case E_NTL_ERROR: // NTL.h
      s = "NTL ERROR";
      break;
    case E_REGEX_ERROR: // regex.h
      snprintf(buf, MAX_CF_ERROR_LEN, "Regex has failed with error code %d", regex_err);
      s = buf;
      break;
    default: // cf
      CF_ERR_LEN = (int) (sizeof(CF_ERR) / sizeof(CF_ERR[0]));
      for(i = 0; i < CF_ERR_LEN; i++) {
        if(CF_ERR[i].err_no == cf_err_no) {
          //strcpy(buf, CF_ERR[i].errmsg);
          //buf[MAX_CF_ERROR_LEN - 1] = '\0'; // ensure null termination
          //s = buf;
          s = CF_ERR[i].errmsg;
          break;
        }
      }
      if(s == NULL) {
        snprintf(buf, MAX_CF_ERROR_LEN, "Unknown CF error %d", cf_err_no);
        s = buf;
      }
      break;
  }
  return s;
}

/**
 * Produces a message to the standard error output, describing the error in 
 * \p cf_err_no. First (if \p s is not \c NULL and <tt>*s</tt> is not a <tt>'\0'</tt>),
 * the argument string \p s is printed, followed by a colon and a blank. Then the
 * message and a new line. If the error originated from an external library, the
 * name of that library is printed right before the error message.
 * 
 * @param[in] s         String to be prepended to the error string.
 * @param[in] cf_err_no CF error code.
 */
void cf_perror(const char *s, cf_errno cf_err_no) {
  if(s != NULL && *s != '\0')
    (void) fprintf(stderr, "%s: ", s);
  switch(cf_err_no) {
    case E_ERRNO_ERROR: (void) fprintf(stderr, "(errno) "); break;
    case E_GCRY_ERROR: (void) fprintf(stderr, "(gcrypt) "); break;
    case E_GMP_ERROR: (void) fprintf(stderr, "(gmp) "); break;
    case E_NTL_ERROR: (void) fprintf(stderr, "(ntl) "); break;
  }
  (void) fprintf(stderr, "%s\n", str_cferror(cf_err_no));
}

void gcry_dump_error(gcry_error_t err) {
  fprintf(stderr, "gcry library failure: %s (source: %s)\n",
                   gcry_strerror(err),
                   gcry_strsource(err));
}
