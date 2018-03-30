#include "random.h"

#define _GNU_SOURCE

#include <unistd.h>
#include <sys/syscall.h>

#define SYS_getrandom 318

ssize_t getrandom(void *buf, size_t buflen, unsigned int flags) {
    return syscall(SYS_getrandom, buf, buflen, flags);
}