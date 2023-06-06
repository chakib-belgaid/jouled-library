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


//  structures  and types definitions
typedef struct
{
    int cpu_num;
    long long timestamp;
    char event_name[50];
    uint64_t value;
} event_t;

typedef struct
{
    int shmid;
    event_t *shared_data;
} slot_t;



//  monitorting functions
void setup_cpu(int cpu_num, const char *events);
void clean_cpu(int cpu_num);
void measure(int cpu_num, int event_code, event_t *event);
void init(int ncpus,int group,int excl);
void terminate();
//  memory functions
slot_t create_shmemroy_slot(key_t shmkey);
int clean_shmemory_slot(slot_t memory_slot);

#endif /* JOULED_H */
