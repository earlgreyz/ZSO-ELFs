#include <curses.h>

#include <stdlib.h>
#include <stdio.h>

#include <sys/ptrace.h>
#include <asm/ptrace.h>

#include <sys/wait.h>
#include <sys/types.h>

#include <signal.h>
#include <unistd.h>
#include <sys/errno.h>

#include "alienos.h"
#include "emulator.h"

#include<sys/reg.h>
#include<sys/user.h>
#include<sys/syscall.h>
#include<string.h>
#include<stdlib.h>
#include <errno.h>

static int alienos_program(int argc, char **argv) {
    ptrace(PTRACE_TRACEME, 0, NULL, NULL);
    if (execv(argv[0], argv) == -1) {
        fprintf(stderr, "Unable to exec %s\n", argv[0]);
        return 1;
    }
    return 0;
}

static void check_or_fail(int err, const char * message) {
    if (err != 0 && errno) {
        fprintf(stderr, message);
        endwin();
        exit(EXIT_FAILURE);
    }
}

static int emulator(pid_t pid) {
    int err, status, syscall_nr;
    int x, y, n, key;
    uint16_t *chars;
    uint32_t rand;
    struct user_regs_struct regs;

    initscr();
    if (initialize_emulator() != 0) {
        fprintf(stderr, "Unable to init emulator\n");
        endwin();
        exit(1);
    }
    fprintf(stderr, "Wait for fork\n");

    wait(&status);
    ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
    fprintf(stderr, "After exec\n");

    do {
        pid = wait(&status);
        if (pid == -1 || WIFEXITED(status)) {
            fprintf(stderr, "AlienOS program finished\n");
            endwin();
            return 0;
        }

        if (WSTOPSIG(status) == SIGTRAP) {
            err = ptrace(PTRACE_GETREGS, pid, NULL, &regs);
            fprintf(stderr, "AlienOS program syscalled %d\n", regs.orig_rax);

            switch (regs.orig_rax) {
                case SYSCALL_END:
                    sys_end(regs.rdi);
                    break;
                case SYSCALL_GETRAND:
                    regs.rax = sys_getrand();
                    err = ptrace(PTRACE_SETREGS, pid, NULL, &regs);
                    check_or_fail(err, "getrand");
                    break;
                case SYSCALL_GETKEY:
                    regs.rax = sys_getkey();
                    err = ptrace(PTRACE_SETREGS, pid, NULL, &regs);
                    check_or_fail(err, "getkey");
                    break;
                case SYSCALL_PRINT:
                    sys_print(regs.rdi, regs.rsi, regs.rdx, regs.r10);
                    refresh();
                    break;
                case SYSCALL_SETCURSOR:
                    sys_setcursor(regs.rdi, regs.rsi);
                    refresh();
                    break;
                default:
                    fprintf(stderr, "Invalid syscall %d\n", regs.orig_rax);
                    endwin();
                    return 127;
            }

            ptrace(PTRACE_SYSEMU, pid, NULL, NULL);
        }

    } while (1);
}

int main(int argc, char *argv[]) {
    pid_t pid;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <program>\n", argv[0]);
        return EXIT_FAILURE;
    }

    switch (pid = fork()) {
        case -1:
            fprintf(stderr, "Usage: %s <program>\n", argv[0]);
            return EXIT_FAILURE;
        case 0:
            return alienos_program(argc - 1, argv + 1);
        default:
            return emulator(pid);
    }
}