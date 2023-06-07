#include <stdio.h>
#include <stdlib.h>
#include "jouled.h"
#include <time.h>

void print_event(event_t event)
{
   printf( "value: %" PRIu64 " ts:%lld \n", event.value, event.timestamp);
//    printf("cpu : %d  event : %s value: %" PRIu64 " ts:%lld \n", event.cpu_num, event.event_name, event.value, event.timestamp);
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
    options_t options;
    options.delay_ns = 490000;
    options.group= 0 ; 
    options.excl = 0 ; 
    cpu_t cpus; 
    cpus.cpu_num = 1 ;
    cpus.events = malloc(sizeof(char *) * 1000);
    // cpus.events= "RAPL_ENERGY_CORES,RAPL_ENERGY_PKG,RAPL_ENERGY_GPU";
    sprintf(cpus.events, "RAPL_ENERGY_CORES,RAPL_ENERGY_PKG,RAPL_ENERGY_GPU");
    cpus.number_of_events = 3 ;
    
    event_t *events = malloc(sizeof(event_t) * 3);
    
    init(1, &cpus, options);
    setup_cpu(1, cpus.events);
    slot_t eve = create_shmemroy_slot(12234);
    event_t *event = eve.shared_data;
    for (int i = 0; i < 100000000; i++)
    {
        // sleep for 1 microsecond
        nanosleep(&tim, NULL);
        measure(cpus.cpu_num, 0, &events[1]);
        measure(cpus.cpu_num, 1, &events[0]);
        measure(cpus.cpu_num, 2, &events[2]);
        // print_events(events, 3);
    }
    clean_shmemory_slot(eve);
    clean_cpu(cpus.cpu_num);

    terminate();
    return 0;
}