#ifndef JOULED_H
#define JOULED_H

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 199309L
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <err.h>
#include <time.h>
#include <perfmon/pfmlib_perf_event.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "perf_util.h"


typedef  uint64_t measure_t; 

typedef struct {
    int group;
    int excl;
    unsigned long   delay_ns;
    key_t           base_shmkey;

} options_t;

typedef struct
{
    int shmid;
    measure_t *shared_data;
} slot_t;



typedef struct {
    int cpu_num;
    char *event_name;
    int event_code;
} event_t;

typedef struct {
    int cpu_num;
    char *events;
    int number_of_events;
} cpu_t;




//  monitorting functions
int setup_perf_cpu(int cpu_num, const char *events);
void clean_perf_cpu(int cpu_num);
measure_t measure_event(int cpu_num, int event_code);
void measure_all();
void init(int ncpus,cpu_t *cpus,options_t options);
void terminate();
//  memory functions
slot_t create_shmemroy_slot(key_t shmkey);
void clean_shmemory_slot(slot_t memory_slot);
void set_memories();
void clean_memories();

//  utility functions
struct timespec convert_ts(unsigned long delay_ns);
measure_t get_time();

#endif /* JOULED_H */
