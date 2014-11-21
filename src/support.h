#pragma once

#include "mpi-cf.h"

#ifdef __cplusplus
extern "C" {
#endif

cf_errno    str_to_size(size_t *res, const char *str, int base);
cf_errno    str_to_uint(uint *res, const char *str, int base);

#ifdef __cplusplus
}
#endif
