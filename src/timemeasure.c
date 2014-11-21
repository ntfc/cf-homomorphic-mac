#include "timemeasure.h"

int is_time_measure = 0;

#ifdef USE_TIME_MEASURE
#include <stdio.h>
#include "cf.h"
#include "logger.h"

ulong timespec_to_ms(struct timespec a) {
  return a.tv_sec*1000 + a.tv_nsec/1000000;
}
ulong rusage_to_ms(struct rusage rus) {
  return rus.ru_utime.tv_sec * 1000 + rus.ru_utime.tv_usec / 1000;
}

ulong elapsed_wall_time_in_ms(struct timespec start, struct timespec end) {
  return timespec_to_ms(end) - timespec_to_ms(start);
}
// returns only the decimal part in microseconds
ulong elapsed_wall_time_rest_in_us(struct timespec start, struct timespec end) {
  return end.tv_nsec/1000 - start.tv_nsec/1000;
}
ulong elapsed_cpu_time_in_ms(struct rusage start, struct rusage end) {
  return rusage_to_ms(end) - rusage_to_ms(start);
}
// returns only the decimal part in microseconds
ulong elapsed_cpu_time_rest_in_us(struct rusage start, struct rusage end) {
  return end.ru_utime.tv_usec - start.ru_utime.tv_usec;
}

ulong cputime(void) {
  struct rusage rus;

  getrusage(RUSAGE_SELF, &rus);

  return rus.ru_utime.tv_sec * 1000 + rus.ru_utime.tv_usec / 1000;
}

void start_measure(cf_time_t *t) {
  if(is_time_measure) {
    if(t != NULL) {
      START_CPU_TIME(t->cpu_start);
      START_WALL_TIME(t->wall_start);
    }
  }
}
void end_measure(cf_time_t *t) {
  if(is_time_measure) {
    if(t != NULL) {
      END_CPU_TIME(t->cpu_end);
      END_WALL_TIME(t->wall_end);
    }
  }
}

void fprint_wall_time(FILE *fp, const cf_time_t t) {
  if(fp == NULL) return;
  ulong wall_ms = 0UL;
  ulong wall_us = 0UL;
  if(is_time_measure) {
    wall_ms = elapsed_wall_time_in_ms(t.wall_start, t.wall_end);
    fprintf(fp, "%lu", wall_ms);
    if(wall_ms == 0UL) { // get the decimal part, in us
      wall_us = elapsed_wall_time_rest_in_us(t.wall_start, t.wall_end);
      if(wall_us != 0UL)
        fprintf(fp, ".%lu", wall_us);
    }
  }
}

void print_measure(const char *s, const cf_time_t t) {
  ulong cpu_ms = 0UL, wall_ms = 0UL;
  ulong cpu_us = 0UL, wall_us = 0UL;
  if(is_time_measure) {
    cpu_ms = elapsed_cpu_time_in_ms(t.cpu_start, t.cpu_end);
    wall_ms = elapsed_wall_time_in_ms(t.wall_start, t.wall_end);
    if(s != NULL && *s != '\0')
      fprintf(stderr, "%s: ", s);
    fprintf(stderr, "wall=%lu", wall_ms);
    if(wall_ms == 0UL) { // get the decimal part, in us
      wall_us = elapsed_wall_time_rest_in_us(t.wall_start, t.wall_end);
      if(wall_us != 0UL)
        fprintf(stderr, ".%.lu", wall_us);
    }
    fprintf(stderr, "ms; cpu=%lu", cpu_ms);
    if(cpu_ms == 0UL) { // get the decimal part, in us
      cpu_us = elapsed_cpu_time_rest_in_us(t.cpu_start, t.cpu_end);
      if(cpu_us != 0UL)
        fprintf(stderr, ".%lu", cpu_us);
    }
    fprintf(stderr, "ms\n");
  }
}

#else
// Nothing here
#endif
