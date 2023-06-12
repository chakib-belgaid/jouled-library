#include "jouled.h"

// global variables for perf
static perf_event_desc_t **all_fds;
static int *num_fds;
static int *monitored_cpus;
//  global variables for monitoring
static event_t *all_events;
static slot_t *all_measures;
static int NCPUS;
static int NEVENTS;

// options
static options_t options;

int setup_perf_cpu(int cpu_num, const char *events)
{
    perf_event_desc_t *fds;
    int i, ret;
    ret = perf_setup_list_events(events, &all_fds[cpu_num], &num_fds[cpu_num]);
    if (ret || (num_fds == 0))
        errx(1, "cannot setup events\n");
    fds = all_fds[cpu_num]; // fds is an array of perf_event_desc_t for the cpu cpu_num

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

    return num_fds[cpu_num];
}

void clean_perf_cpu(int cpu)
{
    perf_event_desc_t *fds = all_fds[cpu];
    for (int i = 0; i < num_fds[cpu]; i++)
        close(fds[i].fd);
    perf_free_fds(fds, num_fds[cpu]);
}

struct timespec convert_ts(unsigned long delay_ns)
{

    struct timespec ts;
    ts.tv_sec = delay_ns / 1000000000;
    ts.tv_nsec = delay_ns % 1000000000;
    return ts;
}

measure_t get_time()
{
    //  return the time in microseconds
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (measure_t)(ts.tv_sec * 1000LL + ts.tv_nsec / 1000LL);
}

measure_t measure_event(int cpu, int event_code)
{
    perf_event_desc_t *fds = all_fds[cpu];

    int ret;
    measure_t val;
    ret = read(fds[event_code].fd, fds[event_code].values, sizeof(fds[event_code].values));
    if (ret != sizeof(fds[event_code].values))
    {
        if (ret == -1)
            err(1, "cannot read event %d:%d", event_code, ret);
        else
            warnx("could not read event%d", event_code);
        return -1;
    }
    val = perf_scale(fds[event_code].values);

    fds[event_code].prev_values[0] = fds[event_code].values[0];
    fds[event_code].prev_values[1] = fds[event_code].values[1];
    fds[event_code].prev_values[2] = fds[event_code].values[2];

    return val;
}

void init(int ncpus, cpu_t *cpus, options_t options_param)
{
    NCPUS = ncpus;
    options = options_param;
    NEVENTS = 0;
    monitored_cpus = calloc(ncpus, sizeof(int));
    //

    all_fds = calloc(ncpus, sizeof(perf_event_desc_t *));
    num_fds = calloc(ncpus, sizeof(int));

    int ret = pfm_initialize();
    if (ret != PFM_SUCCESS)
    {
        errx(1, "libpfm initialization failed: %s\n", pfm_strerror(ret));
    }

    for (int i = 0; i < ncpus; i++)
    {
        NEVENTS += setup_perf_cpu(cpus[i].cpu_num, cpus[i].events);
        monitored_cpus[i] = cpus[i].cpu_num;
    }

    set_memories();
}

void measure_all()
{
    struct timespec tm = convert_ts(options.delay_ns);
    for (;;)
    {
        for (int event_num = 0; event_num < NEVENTS; event_num++)
        {
            *all_measures[event_num].shared_data = measure_event(all_events[event_num].cpu_num, all_events[event_num].event_code);
        }

        *all_measures[NEVENTS].shared_data = get_time();
        nanosleep(&tm, NULL);
    }
}

void terminate()
{
    for (int i = 0; i < NCPUS; i++)
    {
        clean_perf_cpu(i);
    }
    clean_memories();
    pfm_terminate();
}

//  memory functions
slot_t create_shmemroy_slot(key_t shmkey)
{

    slot_t memory_slot;
    memory_slot.shmid = shmget(shmkey, sizeof(measure_t), IPC_CREAT | 0666);
    memory_slot.shared_data = (measure_t *)shmat(memory_slot.shmid, NULL, 0);
    if ((measure_t *)memory_slot.shared_data == (measure_t *)-1)
    {
        perror("shmat");
    }
    return memory_slot;
}

int clean_shmemory_slot(slot_t memory_slot)
{
    // Detach the shared memory segment from our process
    // if (shmdt(memory_slot.shared_data) == -1)
    {
        perror("shmdt");
    return -1;
    }
    // remove the shared memeory segment
    if (shmctl(memory_slot.shmid, IPC_RMID, NULL) == -1)
    {
        perror("shmctl");
        return -1; 
    }
    return 0 ; 
}

void set_memories()
{
    //  set the shared memories
    perf_event_desc_t *fds;

    all_events = calloc(NEVENTS + 1, sizeof(event_t));
    all_measures = calloc(NEVENTS + 1, sizeof(slot_t *));
    int event_index = 0;
    for (int i = 0; i < NCPUS; i++)
    {
        int cpu_num = monitored_cpus[i];
        fds = all_fds[cpu_num];
        for (int eve = 0; eve < num_fds[cpu_num]; eve++)
        {
            all_events[event_index].event_name = calloc(100, sizeof(char));
            sprintf(all_events[event_index].event_name, "%s", fds[eve].name);
            all_events[event_index].cpu_num = cpu_num;
            all_events[event_index].event_code = eve;
            printf("event %d: %s shmkey %d \n", event_index, all_events[event_index].event_name, event_index + options.base_shmkey);
            all_measures[event_index] = create_shmemroy_slot(event_index + options.base_shmkey);
            event_index++;
        }
    }
    //  add one measure  for time
    all_events[NEVENTS].event_name = calloc(100, sizeof(char));
    sprintf(all_events[NEVENTS].event_name, "time");
    all_events[NEVENTS].cpu_num = -1;
    all_measures[NEVENTS] = create_shmemroy_slot(NEVENTS + options.base_shmkey);
    printf("event %d: %s shmkey %d \n", event_index, all_events[event_index].event_name, event_index + options.base_shmkey);
}

void clean_memories()
{
    for (int i = 0; i < NEVENTS + 1; i++)
    {
        clean_shmemory_slot(all_measures[i]);
    }
    free(monitored_cpus);
    free(all_fds);
    free(num_fds);

    free(all_events);
    free(all_measures);
}
