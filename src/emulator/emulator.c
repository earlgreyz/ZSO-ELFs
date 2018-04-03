#define _GNU_SOURCE

#include "emulator.h"
#include "syscall.h"
#include "params.h"
#include "../alienos/alienos.h"
#include "../system/convert.h"
#include "../system/memory.h"

#include <stdio.h>
#include <errno.h>
#include <signal.h>

#include <sys/ptrace.h>
#include <asm/ptrace.h>
#include <sys/wait.h>
#include <sys/prctl.h>

#define IS_ERR(x) ((x) == -1)
#define IS_PTRACE_ERR(x) ((x) == -1 && errno)
#define MIN(x, y) ((x) > (y)? (y): (x))

/// Immediately stops the program execution and prints the error message.
/// \param message error message.
static void emulator_failure(const char *message) {
    fprintf(stderr, "%s\n", message);
    exit(EXIT_ALIENOS_FAIL);
}

/// Initializes arguments in AlienOS process memory.
/// \param child alien process tracer data.
/// \param argc arguments count.
/// \param argv argument values.
/// \return init status (OK|ERR)
static int init_args(struct alien_proc_struct *child, int argc, char *argv[]) {
    off_t params;
    size_t size, args;

    switch (get_params_address(argv[0], &params, &size)) {
        case NO_PARAMS:
            args = 1;
            break;
        case HAS_PARAMS:
            args = (size / 4) + 1;
            break;
        default:
            return -1;
    }

    if (args < (size_t) argc) {
        fprintf(stderr, "Expected at most %lu argument(s), found %d. Discarding.\n", (args - 1), (argc - 1));
    }
    args = MIN(args, (size_t) argc);

    int arg;
    for (size_t i = 1; i < args; i++) {
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

static void emulate(struct alien_proc_struct *child) {
    pid_t pid;
    int status = 0;

    do {
        pid = waitpid(child->pid, &status, 0);
        if (pid == -1 || WIFEXITED(status)) {
            sys_end(WEXITSTATUS(status));
        } else if (WSTOPSIG(status) == SIGTRAP) {
            emulate_syscall(child);
        }
    } while (1);
}

void run_emulator(pid_t pid, int argc, char *argv[]) {
    int status;
    struct alien_proc_struct child = {.pid = pid, .mem = -1};

    // Wait for child to stop on EXECVE
    pid = waitpid(child.pid, &status, 0);
    if (pid == -1 || WIFEXITED(status)) {
        emulator_failure("waitpid");
    }

    if (IS_ERR(child.mem = open_memory(pid))) {
        emulator_failure("open_memory");
    }

    if (IS_ERR(init_args(&child, argc, argv))) {
        close(child.mem);
        emulator_failure("init_args");
    }

    if (start_alienos() != 0) {
        emulator_failure("start_alienos");
    }

    if (IS_PTRACE_ERR(ptrace(PTRACE_SYSCALL, child.pid, NULL, NULL))) {
        end_alienos();
        emulator_failure("PTRACE_SYSCALL");
    }

    emulate(&child);
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
