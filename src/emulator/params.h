#ifndef ZSO_ELFS_PARAMS_H
#define ZSO_ELFS_PARAMS_H

#include <sys/types.h>

#define ERR_PARAMS (-1)
#define HAS_PARAMS (0)
#define NO_PARAMS  (1)

/// Finds the address of PT_PARAMS segment in an ELF64 file.
/// \param filename ELF64 binary file.
/// \param section_start pointer to the result section_start.
/// \param section_size pointer to the result section_size.
/// \return ERR_PARAMS if an error occurs.
/// \return HAS_PARAMS if the ELF file contains a PT_PARAMS segment.
/// \return NO_PARAMS if the ELF file doesn't have a PT_PARAMS segment.
int get_params_address(const char *filename, off_t *section_start, size_t *section_size);

#endif //ZSO_ELFS_PARAMS_H
