#define _GNU_SOURCE
#include "random.h"

#include <unistd.h>
#include <sys/syscall.h>

#define SYS_getrandom 318

ssize_t getrandom(void *buf, size_t buflen, unsigned int flags) {
    if (syscall(SYS_getrandom, buf, buflen, flags) == -1) {
        return 1;
    }
    return 0;
}
