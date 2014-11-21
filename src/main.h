#pragma once
#include <argp.h>
#include "io.h"
#include "keys.h"
#include "cf.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * CF command line program options
*/
#define KEYGEN   "keygen"
#define AUTH     "auth"
#define EVAL     "eval"
#define VRFY     "vrfy"
#define PROG     "prog"
#define MSGEVAL  "msgeval"

struct arguments {
  char *function; // keygen, auth, eval, prof, vrfy or msgeval
  int verbose; // verbose of not
  char *outfile; // where to store the result
  char *labelfile; // label file
  char *arithfile; // arith circ file
  char *progfile; // program file
  char *tagfile; // tags file
  char *messagefile; // messages file
  char *skfile;
  char *ekfile;
  uint secparam; // security param
  int time_measure; // measure time or not
  cf_scheme_e scheme;
  uint d;
};


static struct argp_option options[] = {
  // name,   key arg flags       documentation,                           group
  { KEYGEN,  300, 0,  OPTION_DOC, "Generate keys",                          0 },
  { AUTH,    301, 0,  OPTION_DOC, "Authenticate messages",                  0 },
  { EVAL,    302, 0,  OPTION_DOC, "Homomorphic evaluation",                 0 },
  { VRFY,    303, 0,  OPTION_DOC, "Verify evaluated message",               0 },
  { MSGEVAL, 304, 0,  OPTION_DOC, "Evaluate (original) messages",           0 },
  { 0 }
};

static struct argp_option opts_keygen[] = {
  //name      key  arg     flags   doc                                 group
  { "lambda", 'n', "NUM",  0,     "Security parameter of NUM bits (default = 128)", 0 },
  { "out",    'o', "FILE", 0,     "Output file"                                   , 0 },
  { "scheme", 'c', "NUM",  0,     "Use 1 for CF-1 or 2 for CF-2"                  , 0 },
  { "degree", 'D', "NUM",  0,     "Maximum degree allowed for CF-2"               , 0 },
  { "time",    305, 0,     0,     "Show time measures"                            , 0 },
  { "verbose", 'v', 0,     0,     "Verbose"                                       , 0 },
  { 0 }
};
static struct argp_option opts_auth[] = {
  // name      key  arg        flags      doc                              group
  { "sk",      's', "FILE",      0,    "Secret key file"                     , 0 },
  { "label",   'l', "FILE|VAL",  0,    "Label(s) file or label value"        , 0 },
  { "message", 'm', "FILE|VAL",  0,    "Message(s) file or message value"    , 0 },
  { "out",     'o', "FILE",      0,    "Output file"                         , 0 },
  { "time",    305, 0,           0,    "Show time measures"                  , 0 },
  { "verbose", 'v', 0,           0,    "Verbose"                             , 0 },
  { 0 }
};

static struct argp_option opts_eval[] = {
  // name      key  arg        flags      doc                              group
  { "ek",      'e', "FILE",      0,    "Evaluation key file"                 , 0 },
  { "arith",   'a', "FILE",      0,    "Arithmetic circuit file"             , 0 },
  { "scheme",  'c', "NUM",       0,    "Use 1 for CF-1 or 2 for CF-2"        , 0 },
  { "tag",     't', "FILE|VAL",  0,    "Tag(s) file or tag value"            , 0 },
  { "out",     'o', "FILE",      0,    "Output file"                         , 0 },
  { "time",    305, 0,           0,    "Show time measures"                  , 0 },
  { "verbose", 'v', 0,           0,    "Verbose"                             , 0 },
  { 0 }
};

static struct argp_option opts_vrfy[] = {
  // name      key  arg        flags      doc                              group
  { "sk",      's', "FILE",      0,    "Secret key file"                     , 0 },
  { "label",   'l', "FILE|VAL",  0,    "Label(s) file or label value"        , 0 },
  { "message", 'm', "FILE|VAL",  0,    "Message(s) file or message value"    , 0 },
  { "arith",   'a', "FILE",      0,    "Arithmetic circuit file"             , 0 },
  { "tag",     't', "FILE|VAL",  0,    "Tag(s) file or tag value"            , 0 },
  { "time",    305, 0,           0,    "Show time measures"                  , 0 },
  { "verbose", 'v', 0,           0,    "Verbose"                             , 0 },
  { 0 }
};

static struct argp_option opts_msgeval[] = {
  // name      key  arg        flags      doc                              group }
  { "sk",      'e', "FILE",      0,    "Secret key file"                     , 0 },
  { "message", 'm', "FILE|VAL",  0,    "Message(s) file or message value"    , 0 },
  { "arith",   'a', "FILE",      0,    "Arithmetic circuit file"             , 0 },
  { "out",     'o', "FILE",      0,    "Output file"                         , 0 },
  { "time",    305, 0,           0,    "Show time measures"                  , 0 },
  { "verbose", 'v', 0,           0,    "Verbose"                             , 0 },
  { 0 }
};

static error_t parse_opt(int key, char *arg, struct argp_state *state);
static error_t parse_opt_keygen(int key, char *arg, struct argp_state *state);
static error_t parse_opt_auth(int key, char *arg, struct argp_state *state);
static error_t parse_opt_eval(int key, char *arg, struct argp_state *state);
static error_t parse_opt_vrfy(int key, char *arg, struct argp_state *state);
static error_t parse_opt_msgeval(int key, char *arg, struct argp_state *state);

static char doc[] = "Catalano-Fiore scheme.";
static char keygen_doc[] = "Generate keys";
static char auth_doc[] = "Authenticate message(s)";
static char eval_doc[] = "Homomorphic Evaluation";
static char vrfy_doc[] = "Verification";
static char msgeval_doc[] = "Plain message evaluation";


static struct argp argp_global = { options, parse_opt, NULL, doc , NULL, NULL, NULL };

static struct argp argp_keygen = { opts_keygen, parse_opt_keygen, NULL, keygen_doc , NULL, NULL, NULL };
static struct argp argp_auth = { opts_auth, parse_opt_auth, NULL, auth_doc , NULL, NULL, NULL };
static struct argp argp_eval = { opts_eval, parse_opt_eval, NULL, eval_doc , NULL, NULL, NULL };
static struct argp argp_vrfy = { opts_vrfy, parse_opt_vrfy, NULL, vrfy_doc , NULL, NULL, NULL };
static struct argp argp_msgeval = { opts_msgeval, parse_opt_msgeval, NULL, msgeval_doc , NULL, NULL, NULL };

#ifdef __cplusplus
}
#endif
