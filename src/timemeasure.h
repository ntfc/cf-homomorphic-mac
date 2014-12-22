#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#ifdef USE_TIME_MEASURE
  #include <time.h>
  #include <stdio.h>
  #include <sys/resource.h>
  #include "types.h"

  #define START_MEASURE(a)  \
    cf_time_t a; \
    start_measure(&a)

  #define TIME_MEASURE_ON   (is_time_measure = 1)
  #define TIME_MEASURE_OFF  (is_time_measure = 0)
  
  #define END_MEASURE(a)  end_measure(&a)
  #define PRINT_MEASURE(s, a) print_measure(s, a);
  #define FPRINT_WALL_TIME(fp, t) fprint_wall_time(fp, t);
  
  // deprecate this..
  #define CPU_TOTAL(a)  (elapsed_cpu_time_in_ms(a.cpu_start, a.cpu_end))
  #define WALL_TOTAL(a)  (elapsed_wall_time_in_ms(a.wall_start, a.wall_end))
  
  #define TIME struct timespec
  #define CPU struct rusage

  #define CLOCK_TIME(a) (clock_gettime(CLOCK_MONOTONIC, &a))
  #define START_WALL_TIME(a) (CLOCK_TIME(a))
  #define END_WALL_TIME(a) (CLOCK_TIME(a))

  #define CPU_TIME(a) (getrusage(RUSAGE_SELF, &a))
  #define START_CPU_TIME(a) (CPU_TIME(a))
  #define END_CPU_TIME(a) (CPU_TIME(a))

  typedef struct cf_time_s {
    struct timespec wall_start;
    struct timespec wall_end;
    struct rusage cpu_start;
    struct rusage cpu_end;
  } cf_time_t;

  void start_measure(cf_time_t *t);
  void end_measure(cf_time_t *t);
  void print_measure(const char *s, const cf_time_t t);
  void fprint_wall_time(FILE *fp, const cf_time_t t);

  ulong timespec_to_ms(TIME a);
  /**
   * Computes the difference between start and end wall time, in ms
   */
  ulong elapsed_wall_time_in_ms(TIME start, TIME end);
  ulong elapsed_wall_time_rest_in_us(TIME start, TIME end);
  ulong elapsed_cpu_time_in_ms(CPU start, CPU end);
  ulong elapsed_cpu_time_rest_in_us(CPU start, CPU end);
  /**
   * returns cpu time in ms
   */
  ulong cputime(void);
  
  extern int is_time_measure;
#else
  extern int is_time_measure;
  
  #define TIME_MEASURE_ON     ;
  #define TIME_MEASURE_OFF    ;

  #define START_MEASURE(a)    void *a;a = NULL; a++;
  #define END_MEASURE(a)      
  #define PRINT_MEASURE(s, a) 
  
  #define CPU_TOTAL(a)  (0)
  #define WALL_TOTAL(a)  (0)
#endif
  
#ifdef __cplusplus
}
#endif
