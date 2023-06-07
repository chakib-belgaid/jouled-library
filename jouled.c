#include "jouled.h"

// global variables for perf
static perf_event_desc_t **all_fds;
static int *num_fds;
static int ncpus;
static cpu_t *cpus;

//  global variables for shared memory 
slot_t *all_slots;
// options 
static options_t options;


void setup_cpu(int cpu_num, const char *events)
{   
    perf_event_desc_t *fds;
    int i, ret;

    ret = perf_setup_list_events(events, &all_fds[cpu_num], &num_fds[cpu_num]);
    if (ret || (num_fds == 0))
        errx(1, "cannot setup events\n");
    fds = all_fds[cpu_num]; //fds is an array of perf_event_desc_t for the cpu cpu_num

    fds[0].fd = -1;
    for (i = 0; i < num_fds[cpu_num]; i++)
    {
        fds[i].hw.disabled = options.group ? !i : 1;

        if (options.excl && ((options.group && !i) || (!options.group)))
            fds[i].hw.exclusive = 1;

        fds[i].hw.disabled = options.group ? !i : 1;

        /* request timing information necessary for scaling counts */
        fds[i].hw.read_format = PERF_FORMAT_SCALE;
        fds[i].fd = perf_event_open(&fds[i].hw, -1, cpu_num, (options.group ? fds[0].fd : -1), 0);
        if (fds[i].fd == -1)
            err(1, "cannot attach event to CPU%d %s", cpu_num, fds[i].name);
    }

    if (options.group)
    {
        ret = ioctl(fds[0].fd, PERF_EVENT_IOC_ENABLE, 0);
        if (ret)
            err(1, "cannot get id for event %s\n", fds[0].name);
    }
    else
    {
        for (int i = 0; i < num_fds[cpu_num]; i++)
        {
            ret = ioctl(fds[i].fd, PERF_EVENT_IOC_ENABLE, 0);
            if (ret)
                err(1, "cannot enable event %s\n", fds[i].name);
        }
    }
}





void clean_cpu(int cpu)
{
    perf_event_desc_t *fds = all_fds[cpu];
    for (int i = 0; i < num_fds[cpu]; i++)
        close(fds[i].fd);
    perf_free_fds(fds, num_fds[cpu]);
}


struct timespec convert_ts(unsigned long  delay_ns){

    struct timespec ts;
    ts.tv_sec = delay_ns / 1000000000;
    ts.tv_nsec = delay_ns % 1000000000;
    return ts;

}


long long get_time()
{

    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (long long)(ts.tv_sec * 1000000000LL + ts.tv_nsec);
}

void measure(int cpu, int event_code, event_t *event)
{
    perf_event_desc_t *fds = all_fds[cpu];
    int ret;
    uint64_t val;
    ret = read(fds[event_code].fd, fds[event_code].values, sizeof(fds[event_code].values));
    if (ret != sizeof(fds[event_code].values))
    {
        if (ret == -1)
            err(1, "cannot read event %d:%d", event_code, ret);
        else
            warnx("could not read event%d", event_code);
    }
    val = perf_scale(fds[event_code].values);
    event->cpu_num = cpu;
    event->value = val;
    event->timestamp = get_time();
    strcpy(event->event_name, fds[event_code].name);
    fds[event_code].prev_values[0] = fds[event_code].values[0];
    fds[event_code].prev_values[1] = fds[event_code].values[1];
    fds[event_code].prev_values[2] = fds[event_code].values[2];
}

void init(int ncpus,cpus_t *cpus , options_t options)
{   ncpus = ncpus;  
    options = options;
    for (int i = 0; i < ncpus; i++)
        {
            setup_cpu(i, cpus[i].events);

        }

    all_fds = calloc(ncpus, sizeof(perf_event_desc_t *));
    num_fds = calloc(ncpus, sizeof(int));
    int ret = pfm_initialize();
    if (ret != PFM_SUCCESS)
    {
        errx(1, "libpfm initialization failed: %s\n", pfm_strerror(ret));
    }
    
}

void terminate()
{ 
//   for (int i = 0; i < ncpus; i++)
//         clean_cpu(i);

    pfm_terminate();
}

//  memory functions

slot_t create_shmemroy_slot(key_t shmkey)
{

    slot_t memory_slot;
    memory_slot.shmid = shmget(shmkey, sizeof(event_t), IPC_CREAT | 0666);
    memory_slot.shared_data = (event_t *)shmat(memory_slot.shmid, NULL, 0);
    if ((event_t *)memory_slot.shared_data == (event_t *)-1)
    {
        perror("shmat");
    }
    return memory_slot;
}

int clean_shmemory_slot(slot_t memory_slot)
{
    // Detach the shared memory segment from our process
    if (shmdt(memory_slot.shared_data) == -1)
    {
        perror("shmdt");
        return 1;
    }
    // remove the shared memeory segment
    if (shmctl(memory_slot.shmid, IPC_RMID, NULL) == -1)
    {
        perror("shmctl");
        return 1;
    }
    return 0;
}
