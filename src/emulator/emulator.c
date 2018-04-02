#define _GNU_SOURCE

#include "emulator.h"
#include "emulate.h"
#include "alienos.h"
#include "../system/convert.h"

#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>

#include <sys/ptrace.h>
#include <asm/ptrace.h>
#include <sys/wait.h>
#include <sys/prctl.h>
#include <fcntl.h>
#include <unistd.h>

#define IS_ERR(x) ((x) == -1)
#define IS_PTRACE_ERR(x) ((x) == -1 && errno)

static void system_failure(const char *message) {
    end_alienos();
    fprintf(stderr, "%s\n", message);
    exit(EXIT_ALIENOS_FAIL);
}

/// Opens child memory descriptor
/// \param pid alien process pid.
/// \return descriptor of the child process memory or error (-1)
static int open_proc_mem(pid_t pid) {
    char file[64];
    if (sprintf(file, "/proc/%ld/mem", (long) pid) == -1) {
        return -1;
    }
    return open(file, O_RDWR);
}

/// Initializes arguments in AlienOS process memory.
/// \param child alien process tracer data.
/// \param argc arguments count.
/// \param argv argument values.
/// \return init status (OK|ERR)
static int init_args(struct alien_proc_struct *child, int argc, char *argv[]) {
    int arg;
    off_t params = 0x3133a6fc;

    for (int i = 1; i < argc; i++) {
        if (IS_ERR(str_to_int(argv[i], &arg))) {
            return -1;
        }

        off_t address = params + (off_t) (i - 1) * (off_t) sizeof(arg);
        if (pwrite(child->mem, &arg, sizeof(arg), address) != (ssize_t) sizeof(arg)) {
            return -1;
        }
    }
    return 0;
}

void run_emulator(pid_t pid, int argc, char *argv[]) {
    int status;
    struct alien_proc_struct child = {.pid = pid, .mem = -1};

    if (start_alienos() != 0) {
        system_failure("Unable to start AlienOS emulator");
    }

    // Wait for child to stop on EXECVE
    pid = waitpid(child.pid, &status, 0);
    if (pid == -1 || WIFEXITED(status)) {
        system_failure("AlienOS processed finished before it was started");
    }

    if (IS_PTRACE_ERR(ptrace(PTRACE_SYSCALL, pid, NULL, NULL))) {
        system_failure("PTRACE_SYSCALL");
    }

    if (IS_ERR(child.mem = open_proc_mem(pid))) {
        system_failure("open_proc_mem");
    }

    if (IS_ERR(init_args(&child, argc, argv))) {
        emulation_failure(&child, "init_args");
    }

    do {
        pid = waitpid(child.pid, &status, 0);
        if (pid == -1 || WIFEXITED(status)) {
            sys_end(WEXITSTATUS(status));
        } else if (WSTOPSIG(status) == SIGTRAP) {
            emulate_syscall(&child);
            if (IS_PTRACE_ERR(ptrace(PTRACE_SYSEMU, pid, NULL, NULL))) {
                emulation_failure(&child, "PTRACE_SYSEMU");
            }
        }
    } while (1);
}

void run_program(char *program) {
    char *const argv[] = {NULL};

    // Die when parent is killed.
    if (prctl(PR_SET_PDEATHSIG, SIGHUP) == -1) {
        fprintf(stderr, "PR_SET_PDEATHSIG\n");
        exit(EXIT_ALIENOS_FAIL);
    }

    if (IS_PTRACE_ERR(ptrace(PTRACE_TRACEME, 0, NULL, NULL))) {
        fprintf(stderr, "PTRACE_TRACEME\n");
        exit(EXIT_ALIENOS_FAIL);
    }

    if (execv(program, argv) == -1) {
        fprintf(stderr, "Unable to exec %s\n", program);
        exit(EXIT_ALIENOS_FAIL);
    }
}
