#include "memory.h"

#include <stdio.h>
#include <fcntl.h>

int open_memory(pid_t pid) {
    char file[64];
    if (sprintf(file, "/proc/%ld/mem", (long) pid) == -1) {
        return -1;
    }
    return open(file, O_RDWR);
}
