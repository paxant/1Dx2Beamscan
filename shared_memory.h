#ifndef SHARED_MEMORY_H
#define SHARED_MEMORY_H

#include <complex.h>
#include "beam_shared.h"
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#define SHM_NAME "/beamscan_shm"

typedef struct
{
    int ready;
    double complex SignalArray[SNAPSHOTS][NUMBER_X * NUMBER_Y];
} SharedSignalArray;

#endif