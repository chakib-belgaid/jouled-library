#include <stdio.h>
#include <stdlib.h>
#include "jouled.h"
#include <time.h>

int main()
{
    struct timespec tim;
    tim.tv_sec = 0;
    tim.tv_nsec = 490000L;
    options_t options;
    options.delay_ns = 490000;
    options.group = 0;
    options.excl = 0;
    options.base_shmkey = 0x12334;
    cpu_t cpus;
    cpus.cpu_num = 2;
    cpus.events = malloc(sizeof(char *) * 1000);
    sprintf(cpus.events, "RAPL_ENERGY_CORES,RAPL_ENERGY_PKG,RAPL_ENERGY_GPU");

    cpus.number_of_events = 3;

    init(1, &cpus, options);


    for (int i = 0; i < 100000000; i++)
    {
        // sleep for 1 microsecond
        nanosleep(&tim, NULL);

        measure_all();
        // print_events(events, 3);
    }

    terminate();
    return 0;
}