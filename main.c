#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "emulator.h"


int main(int argc, char *argv[]) {
    pid_t pid;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <program>\n", argv[0]);
        return EXIT_FAILURE;
    }

    switch (pid = fork()) {
        case -1:
            fprintf(stderr, "Failed to fork\n");
            return EXIT_FAILURE;
        case 0:
            run_program(argc - 1, argv + 1);
        default:
            run_emulator(pid);
    }
}
