#include "emulator.h"

#include <stdio.h>
#include <errno.h>
#include <curses.h>
#include <signal.h>
#include <unistd.h>

#include <sys/ptrace.h>
#include <asm/ptrace.h>
#include <sys/wait.h>
#include <sys/reg.h>
#include <sys/user.h>
#include <sys/syscall.h>
#include <sys/prctl.h>

#include "alienos.h"

static void fail(const char * message) {
    end_window();
    fprintf(stderr, message);
    exit(EXIT_FAILURE);
}

static void fail_when_err(int err, const char *message) {
    if (err != 0 && errno) {
        fail(message);
    }
}

static void emulate_end(pid_t pid, struct user_regs_struct *regs) {
    sys_end(regs->rdi);
}

static void emulate_getrand(pid_t pid, struct user_regs_struct *regs) {
    regs->rax = sys_getrand();
    int err = ptrace(PTRACE_SETREGS, pid, NULL, regs);
    fail_when_err(err, "getrand");
}

static void emulate_getkey(pid_t pid, struct user_regs_struct *regs) {
    regs->rax = sys_getkey();
    int err = ptrace(PTRACE_SETREGS, pid, NULL, regs);
    fail_when_err(err, "getkey");
}

static int emulate_print(pid_t pid, struct user_regs_struct *regs) {
    int result;
    uint16_t * buffer = (uint16_t *)malloc(sizeof(uint16_t) * regs->r10);
    if (buffer == NULL) {
        fail("malloc");
    }

    for (int i = 0; i < regs->r10; i++) {
        result = ptrace(PTRACE_PEEKTEXT, pid, regs->rdx + sizeof(uint16_t) * i, NULL);
        if (result != 0 && errno) {
            free(buffer);
            fail("peek_text");
        }
        *(buffer + i) = (uint16_t) result;
    }

    sys_print(regs->rdi, regs->rsi, buffer, regs->r10);
    refresh();

    free(buffer);
}

static void emulate_setcursor(pid_t pid, struct user_regs_struct *regs) {
    sys_setcursor(regs->rdi, regs->rsi);
    refresh();
}

static void emulate_syscall(pid_t pid) {
    struct user_regs_struct regs;

    int err = ptrace(PTRACE_GETREGS, pid, NULL, &regs);
    fail_when_err(err, "ptrace_getregs");

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
            end_window();
            fprintf(stderr, "Invalid syscall %d\n", regs.orig_rax);
            exit(127);
    }
}

void run_emulator(pid_t pid) {
    int status;

    if (start_window() != OK) {
        fail("Unable to start AlienOS window");
    }

    // Allow the tracee to execve.
    wait(&status);
    ptrace(PTRACE_SYSCALL, pid, NULL, NULL);

    do {
        pid = wait(&status);
        if (pid == -1 || WIFEXITED(status)) {
            end_window();
            exit(EXIT_SUCCESS);
        } else if (WSTOPSIG(status) == SIGTRAP) {
            emulate_syscall(pid);
            ptrace(PTRACE_SYSEMU, pid, NULL, NULL);
        }
    } while (1);
}

void run_program(int argc, char *argv[]) {
    // Die when parent is killed.
    prctl(PR_SET_PDEATHSIG, SIGHUP);
    ptrace(PTRACE_TRACEME, 0, NULL, NULL);

    if (execv(argv[0], argv) == -1) {
        fprintf(stderr, "Unable to exec %s\n", argv[0]);
        exit(EXIT_FAILURE);
    }
}
