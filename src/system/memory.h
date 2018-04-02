#ifndef ZSO_ELFS_MEMORY_H
#define ZSO_ELFS_MEMORY_H

#include <sys/types.h>

/// Opens the descriptor to the process memory.
/// \param pid process id.
/// \return descriptor of the child process memory or error (-1)
int open_memory(pid_t pid);

#endif //ZSO_ELFS_MEMORY_H
