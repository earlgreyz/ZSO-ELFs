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

#define IS_PTRACE_ERR(x) ((x) == -1 && errno)
#define SUCCESS_OR_RET(x) if ((x) == -1) return -1;

static void emulation_failure(const char *message) {
    end_alienos();
    fprintf(stderr, "%s\n", message);
    exit(EXIT_ALIENOS_FAIL);
}

static void emulate_end(__attribute__((unused)) pid_t pid, struct user_regs_struct *regs) {
    sys_end((int) regs->rdi);
}

static void emulate_getrand(pid_t pid, struct user_regs_struct *regs) {
    regs->rax = sys_getrand();
    if (IS_PTRACE_ERR(ptrace(PTRACE_SETREGS, pid, NULL, regs))) {
        emulation_failure("PTRACE_SETREGS");
    }
}

static void emulate_getkey(pid_t pid, struct user_regs_struct *regs) {
    regs->rax = (long long unsigned int) sys_getkey();
    if (IS_PTRACE_ERR(ptrace(PTRACE_SETREGS, pid, NULL, regs))) {
        emulation_failure("PTRACE_SETREGS");
    }
}

static int read_memory(pid_t pid, void *buffer, size_t buffer_len, off_t address) {
    int fd;
    char file[64];

    SUCCESS_OR_RET(sprintf(file, "/proc/%ld/mem", (long) pid))
    SUCCESS_OR_RET(fd = open(file, O_RDWR))

    if (pread(fd, buffer, buffer_len, address) != (ssize_t) buffer_len) {
        (void) close(fd);
        return -1;
    }

    SUCCESS_OR_RET(close(fd))
    return 0;
}

static void emulate_print(pid_t pid, struct user_regs_struct *regs) {
    size_t buffer_len = sizeof(uint16_t) * regs->r10;
    uint16_t *buffer = (uint16_t *) malloc(buffer_len);
    if (buffer == NULL) {
        emulation_failure("malloc");
    }

    if (read_memory(pid, buffer, buffer_len, (off_t) regs->rdx) == -1) {
        free(buffer);
        emulation_failure("read_memory");
    }

    sys_print((int) regs->rdi, (int) regs->rsi, buffer, (int) regs->r10);
    free(buffer);
}

static void emulate_setcursor(__attribute__((unused)) pid_t pid, struct user_regs_struct *regs) {
    sys_setcursor((int) regs->rdi, (int) regs->rsi);
}

static void emulate_syscall(pid_t pid) {
    struct user_regs_struct regs;

    long err = ptrace(PTRACE_GETREGS, pid, NULL, &regs);
    if (IS_PTRACE_ERR(err)) {
        emulation_failure("PTRACE_GETREGS");
    }

    switch (regs.orig_rax) {
        case SYSCALL_END:
            emulate_end(pid, &regs);
            break;
        case SYSCALL_GETRAND:
            emulate_getrand(pid, &regs);
            break;
        case SYSCALL_GETKEY:
            emulate_getkey(pid, &regs);
            break;
        case SYSCALL_PRINT:
            emulate_print(pid, &regs);
            break;
        case SYSCALL_SETCURSOR:
            emulate_setcursor(pid, &regs);
            break;
        default:
            end_alienos();
            fprintf(stderr, "Invalid syscall %llu\n", regs.orig_rax);
            exit(EXIT_ALIENOS_FAIL);
    }
}

void run_emulator(pid_t pid, int argc, char *argv[]) {
    int status;
    if (start_alienos() != 0) {
        emulation_failure("Unable to start AlienOS emulator");
    }

    // Allow the tracee to execve.
    (void) wait(&status);
    if (IS_PTRACE_ERR(ptrace(PTRACE_SYSCALL, pid, NULL, NULL))) {
        emulation_failure("PTRACE_SYSCALL");
    }

    do {
        pid = wait(&status);
        if (pid == -1 || WIFEXITED(status)) {
            sys_end(status);
        } else if (WSTOPSIG(status) == SIGTRAP) {
            emulate_syscall(pid);
            if (IS_PTRACE_ERR(ptrace(PTRACE_SYSEMU, pid, NULL, NULL))) {
                emulation_failure("PTRACE_SYSEMU");
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
