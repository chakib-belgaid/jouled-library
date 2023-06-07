#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "monitor.c"

void test_create_shmemory_slot()
{
    key_t key = 1234;
    slot_t slot = create_shmemroy_slot(key);
    if (slot.shmid == -1)
    {
        printf("Test failed: create_shmemory_slot() returned -1\n");
        return;
    }
    printf("Test passed: create_shmemory_slot() returned %d\n", slot.shmid);
    clean_shmemory_slot(slot);
}

void test_clean_shmemory_slot()
{
    key_t key = 1234;
    slot_t slot = create_shmemroy_slot(key);
    int result = clean_shmemory_slot(slot);
    if (result != 0)
    {
        printf("Test failed: clean_shmemory_slot() returned %d\n", result);
        return;
    }
    printf("Test passed: clean_shmemory_slot() returned %d\n", result);
}

int main()
{
    test_create_shmemory_slot();
    test_clean_shmemory_slot();
    return 0;
}