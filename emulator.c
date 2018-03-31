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

#define EXIT_INVALID_INSTR 127
#define IS_PTRACE_ERR(x) ((x) == -1 && errno)

static void emulation_failure(const char *message) {
    end_alienos();
    fprintf(stderr, message);
    exit(EXIT_FAILURE);
}

static void emulate_end(pid_t pid, struct user_regs_struct *regs) {
    sys_end(regs->rdi);
}

static void emulate_getrand(pid_t pid, struct user_regs_struct *regs) {
    regs->rax = sys_getrand();
    int err = ptrace(PTRACE_SETREGS, pid, NULL, regs);
    if (IS_PTRACE_ERR(err)) {
        emulation_failure("PTRACE_SETREGS");
    }
}

static void emulate_getkey(pid_t pid, struct user_regs_struct *regs) {
    regs->rax = sys_getkey();
    int err = ptrace(PTRACE_SETREGS, pid, NULL, regs);
    if (IS_PTRACE_ERR(err)) {
        emulation_failure("PTRACE_SETREGS");
    }
}

static int emulate_print(pid_t pid, struct user_regs_struct *regs) {
    int result;
    uint16_t * buffer = (uint16_t *)malloc(sizeof(uint16_t) * regs->r10);
    if (buffer == NULL) {
        emulation_failure("malloc");
    }

    for (int i = 0; i < regs->r10; i++) {
        result = ptrace(PTRACE_PEEKTEXT, pid, regs->rdx + sizeof(uint16_t) * i, NULL);
        if (IS_PTRACE_ERR(result)) {
            free(buffer);
            emulation_failure("PTRACE_PEEKTEXT");
        }
        *(buffer + i) = (uint16_t) result;
    }

    sys_print(regs->rdi, regs->rsi, buffer, regs->r10);
    free(buffer);
}

static void emulate_setcursor(pid_t pid, struct user_regs_struct *regs) {
    sys_setcursor(regs->rdi, regs->rsi);
}

static void emulate_syscall(pid_t pid) {
    struct user_regs_struct regs;

    int err = ptrace(PTRACE_GETREGS, pid, NULL, &regs);
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
            fprintf(stderr, "Invalid syscall %d\n", regs.orig_rax);
            exit(EXIT_INVALID_INSTR);
    }
}

void run_emulator(pid_t pid) {
    int status, err;
    if (start_alienos() != 0) {
        emulation_failure("Unable to start AlienOS emulator");
    }

    // Allow the tracee to execve.
    (void) wait(&status);
    err = ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
    if (IS_PTRACE_ERR(err)) {
        emulation_failure("PTRACE_SYSCALL");
    }

    do {
        pid = wait(&status);
        if (pid == -1 || WIFEXITED(status)) {
            sys_end(status);
        } else if (WSTOPSIG(status) == SIGTRAP) {
            emulate_syscall(pid);
            err = ptrace(PTRACE_SYSEMU, pid, NULL, NULL);
            if (IS_PTRACE_ERR(err)) {
                emulation_failure("PTRACE_SYSEMU");
            }
        }
    } while (1);
}

void run_program(int argc, char *argv[]) {
    // Die when parent is killed.
    if (prctl(PR_SET_PDEATHSIG, SIGHUP) == -1) {
        fprintf(stderr, "PR_SET_PDEATHSIG");
        exit(EXIT_FAILURE);
    }

    if (IS_PTRACE_ERR(ptrace(PTRACE_TRACEME, 0, NULL, NULL))) {
        fprintf(stderr, "PTRACE_TRACEME");
        exit(EXIT_FAILURE);
    }

    if (execv(argv[0], argv) == -1) {
        fprintf(stderr, "Unable to exec %s\n", argv[0]);
        exit(EXIT_FAILURE);
    }
}
