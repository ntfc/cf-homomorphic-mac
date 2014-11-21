#pragma once

#include <stdio.h>
#include "io.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LOG_PRINT(...)                    \
  do {                                    \
    if(is_verbose == 1) {                 \
      fprintf(stderr, __VA_ARGS__);       \
    }                                     \
  } while(0)

#define LOG_MPI(mpi, ...)                 \
  do {                                    \
    if(is_verbose == 1) {                 \
      fprintf(stderr, __VA_ARGS__);       \
      cf_mpi_dump(mpi);                   \
      fprintf(stderr, "\n");              \
    }                                     \
  } while(0)

#define LOG_DUMP(dump_fun, ...)           \
  do {                                    \
    if(is_verbose == 1) {                 \
      fprintf(stderr, __VA_ARGS__);       \
      dump_fun;                           \
      fprintf(stderr, "\n");              \
    }                                     \
  } while(0)

extern int is_verbose;

#define VERBOSE_ON  (is_verbose = 1)
#define VERBOSE_OFF (is_verbose = 0)

#ifdef __cplusplus
}
#endif
