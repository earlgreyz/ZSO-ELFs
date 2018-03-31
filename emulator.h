#ifndef ZSO_ELFS_EMULATOR_H
#define ZSO_ELFS_EMULATOR_H

#include <stdlib.h>
#include <sys/types.h>


/// Connects to process as an AlienOS emulator.
/// \param pid child process
void run_emulator(pid_t pid);

/// Runs alienOS program.
/// \param argc arguments count.
/// \param argv argument values.
void run_program(int argc, char *argv[]);


#endif // ZSO_ELFS_EMULATOR_H
