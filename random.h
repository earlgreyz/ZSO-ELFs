#ifndef ZSO_ELFS_RANDOM_H
#define ZSO_ELFS_RANDOM_H

#include <sys/types.h>

ssize_t getrandom(void *buf, size_t buflen, unsigned int flags);

#endif
