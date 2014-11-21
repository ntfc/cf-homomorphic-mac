#include <stdio.h>
#include <limits.h> // for ULONG_MAX
#include "support.h"

cf_errno str_to_size(size_t *res, const char *str, int base) {
  unsigned long int r;
  size_t i;
  for(i = 0; str[i] != '\0'; i++) {
    if(str[i] < 48 || str[i] > 57) // ie not a number char
      return E_INVAL_NUM;
  }
  r= strtoul(str, NULL, base);
  if(r == ULONG_MAX || r > ((size_t)-1)) {
    //DEBUG_PRINT("error reading string '%s' to size_t\n", str);
    return E_RANGE;
  }
  *res = (size_t)r;
  return E_NOERR;
}

cf_errno str_to_uint(uint *res, const char *str, int base) {
  unsigned long int r;
  size_t i;
  for(i = 0; str[i] != '\0'; i++) {
    if((int)str[i] < 48 || (int)str[i] > 57) // ie not a number char
      return E_INVAL_NUM;
  }
  r = strtoul(str, NULL, base);
  if(r == ULONG_MAX || r > ((uint)-1)) {
    return E_RANGE;
  }
  *res = (uint)r;
  return E_NOERR;
}

