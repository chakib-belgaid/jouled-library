#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>
#include "jouled.c"

void test_create_shmemory_slot()
{
    key_t key = ftok("/tmp", 'a');
    slot_t slot = create_shmemroy_slot(key);
    if (slot.shmid == -1)
    {
        printf("test_create_shmemory_slot: Failed to create shared memory segment\n");
        exit(1);
    }
    printf("test_create_shmemory_slot: Shared memory segment created successfully\n");
    clean_shmemory_slot(slot);
}

void test_clean_shmemory_slot()
{
    key_t key = ftok("/tmp", 'a');
    slot_t slot = create_shmemroy_slot(key);
    int result = clean_shmemory_slot(slot);
    if (result != 0)
    {
        printf("test_clean_shmemory_slot: Failed to clean shared memory segment\n");
        exit(1);
    }
    printf("test_clean_shmemory_slot: Shared memory segment cleaned successfully\n");
}

int main()
{
    test_create_shmemory_slot();
    test_clean_shmemory_slot();
    return 0;
}