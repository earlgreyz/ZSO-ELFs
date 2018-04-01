#define _GNU_SOURCE

#include "emulator.h"
#include "alienos.h"

#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <ncurses.h>

#include <sys/ptrace.h>
#include <asm/ptrace.h>
#include <sys/wait.h>
#include <sys/reg.h>
#include <sys/user.h>
#include <sys/syscall.h>
#include <sys/prctl.h>
#include <fcntl.h>
#include <unistd.h>

#define IS_ERR(x) ((x) == -1)
#define IS_PTRACE_ERR(x) ((x) == -1 && errno)

/// AlienOS process information for the tracer.
struct alien_proc_struct {
    pid_t pid;
    int mem_fd;
};

/// Prints an error and exits.
/// \param message error message.
static void system_failure(const char *message) {
    end_alienos();
    fprintf(stderr, "%s\n", message);
    exit(EXIT_ALIENOS_FAIL);
}

/// Prints an error and exits cleaning up data opened by the tracer.
/// \param child alien process tracer data.
/// \param message error message.
static void emulation_failure(struct alien_proc_struct *child, const char *message) {
    close(child->mem_fd);
    system_failure(message);
}

/// Emulates sys_end syscall.
/// \param child alien process tracer data.
/// \param regs alien process registers data.
static void emulate_end(struct alien_proc_struct *child, struct user_regs_struct *regs) {
    close(child->mem_fd);
    sys_end((int) regs->rdi);
}

/// Emulates sys_getrand syscall.
/// \param child alien process tracer data.
/// \param regs alien process registers data.
static void emulate_getrand(struct alien_proc_struct *child, struct user_regs_struct *regs) {
    uint32_t value;
    if (IS_ERR(sys_getrand(&value))) {
        emulation_failure(child, "sys_getrand");
    }
    regs->rax = value;

    if (IS_PTRACE_ERR(ptrace(PTRACE_SETREGS, child->pid, NULL, regs))) {
        emulation_failure(child, "PTRACE_SETREGS");
    }
}

/// Emulates sys_getkey syscall.
/// \param child alien process tracer data.
/// \param regs alien process registers data.
static void emulate_getkey(struct alien_proc_struct *child, struct user_regs_struct *regs) {
    regs->rax = (long long unsigned int) sys_getkey();
    if (IS_PTRACE_ERR(ptrace(PTRACE_SETREGS, child->pid, NULL, regs))) {
        emulation_failure(child, "PTRACE_SETREGS");
    }
}

/// Emulates sys_print syscall.
/// \param child alien process tracer data.
/// \param regs alien process registers data.
static void emulate_print(struct alien_proc_struct *child, struct user_regs_struct *regs) {
    size_t buffer_len = sizeof(uint16_t) * regs->r10;
    uint16_t *buffer = (uint16_t *) malloc(buffer_len);
    if (buffer == NULL) {
        emulation_failure(child, "malloc");
    }

    if (pread(child->mem_fd, buffer, buffer_len, (off_t) regs->rdx) != (ssize_t) buffer_len) {
        free(buffer);
        emulation_failure(child, "pread");
    }

    if (IS_ERR(sys_print((int) regs->rdi, (int) regs->rsi, buffer, (int) regs->r10))) {
        free(buffer);
        emulation_failure(child, "sys_print");
    }
    free(buffer);
}

/// Emulates sys_setcursor syscall.
/// \param child alien process tracer data.
/// \param regs alien process registers data.
static void emulate_setcursor(struct alien_proc_struct *child, struct user_regs_struct *regs) {
    if (IS_ERR(sys_setcursor((int) regs->rdi, (int) regs->rsi))) {
        emulation_failure(child, "sys_setcursor");
    }
}

/// Emulates alien process syscall.
/// \param child alien process tracer data.
static void emulate_syscall(struct alien_proc_struct *child) {
    struct user_regs_struct regs;

    long err = ptrace(PTRACE_GETREGS, child->pid, NULL, &regs);
    if (IS_PTRACE_ERR(err)) {
        emulation_failure(child, "PTRACE_GETREGS");
    }

    switch (regs.orig_rax) {
        case SYSCALL_END:
            emulate_end(child, &regs);
            break;
        case SYSCALL_GETRAND:
            emulate_getrand(child, &regs);
            break;
        case SYSCALL_GETKEY:
            emulate_getkey(child, &regs);
            break;
        case SYSCALL_PRINT:
            emulate_print(child, &regs);
            break;
        case SYSCALL_SETCURSOR:
            emulate_setcursor(child, &regs);
            break;
        default:
            emulation_failure(child, "unknown syscall");
    }
}

static int map_child_mem(pid_t pid) {
    char file[64];
    if (sprintf(file, "/proc/%ld/mem", (long) pid) == -1) {
        return -1;
    }
    return open(file, O_RDWR);
}

void run_emulator(pid_t pid, int argc, char *argv[]) {
    int status;
    struct alien_proc_struct child = { .pid = pid, .mem_fd = -1 };

    if (start_alienos() != 0) {
        system_failure("Unable to start AlienOS emulator");
    }

    // Allow the tracee to execve.
    (void) wait(&status);
    if (IS_PTRACE_ERR(ptrace(PTRACE_SYSCALL, pid, NULL, NULL))) {
        system_failure("PTRACE_SYSCALL");
    }

    // Open tracee memory
    if (IS_ERR(child.mem_fd = map_child_mem(pid))) {
        system_failure("map_child_mem");
    }

    do {
        pid = wait(&status);
        if (pid == -1 || WIFEXITED(status)) {
            sys_end(status);
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
