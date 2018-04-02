#ifndef ZSO_ELFS_PARAMS_H
#define ZSO_ELFS_PARAMS_H

#include <sys/types.h>

#define ERR_PARAMS (-1)
#define HAS_PARAMS (0)
#define NO_PARAMS  (1)

int get_params_address(const char *filename, off_t *section_start, size_t *section_size);

#endif //ZSO_ELFS_PARAMS_H
