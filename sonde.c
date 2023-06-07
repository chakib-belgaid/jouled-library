#include <stdio.h>
#include <stdlib.h>
#include "jouled.h"
#include <time.h>

void print_event(event_t event)
{
    printf("cpu : %d  event : %s value: %" PRIu64 " ts:%lld \n", event.cpu_num, event.event_name, event.value, event.timestamp);
}

void print_events(event_t *events, int number_of_events)
{
    for (int i = 0; i < number_of_events; i++)
    {
        print_event(events[i]);
    }
}



int main()
{
     struct timespec tim;
     tim.tv_sec = 0;
    tim.tv_nsec = 490000L;

    int cpu = 1;
    char *evets = "RAPL_ENERGY_CORES,RAPL_ENERGY_PKG,RAPL_ENERGY_GPU";
    event_t *events = malloc(sizeof(event_t) * 3);
    init(1,0,0);

    setup_cpu(cpu, evets);
    slot_t eve = create_shmemroy_slot(12234);
    event_t *event = eve.shared_data;
    for (int i = 0; i < 100000000; i++)
    {
        // sleep for 1 microsecond
        nanosleep(&tim, NULL);
        measure(cpu, 0, &events[1]);
        measure(cpu, 1, &events[0]);
        measure(cpu, 2, &events[2]);
        // print_events(events, 3);
    }
    clean_shmemory_slot(eve);
    clean_cpu(cpu);

    terminate();
    return 0;
}