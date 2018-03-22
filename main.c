#include <curses.h>

#include <stdlib.h>
#include <stdio.h>

#include <sys/ptrace.h>
#include <asm/ptrace.h>

#include <sys/wait.h>
#include <sys/types.h>

#include <signal.h>
#include <unistd.h>

#include "alienos.h"
#include "emulator.h"

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

static int emulator(pid_t program) {
    int err, status, syscall_nr;

    initscr();
    if (initialize_emulator() != 0) {
        fprintf(stderr, "Unable to init emulator\n");
        endwin();
        exit(1);
    }

    do {
        if (wait(&status) == -1 || WIFEXITED(status)) {
            fprintf(stderr, "AlienOS program finished\n");
            endwin();
            return 0;
        }

        if (WSTOPSIG(status) == SIGTRAP) {
            syscall_nr = ptrace(PTRACE_PEEKDATA, program, ORIG_RAX, NULL);
            fprintf(stderr, "AlienOS program syscalled %d\n", syscall_nr);

            switch (syscall_nr) {
                case SYSCALL_END:
                    err = ptrace(PTRACE_PEEKDATA, program, ORIG_RDI, NULL);
                    sys_end(err);
                    break;
                case SYSCALL_GETRAND:
                    uint32_t rand = sys_getrand();
                    err = ptrace(PTRACE_POKEDATA, program, ORIG_RAX, rand);
                    check_or_fail(err, "getrand");
                    break;
                case SYSCALL_GETKEY:
                    int key = sys_getkey();
                    err = ptrace(PTRACE_POKEDATA, program, ORIG_RAX, key);
                    check_or_fail(err, "getkey");
                    break;
                case SYSCALL_PRINT:
                    int x = ptrace(PTRACE_PEEKDATA, program, ORIG_RDI, NULL);
                    check_or_fail(x, "print (x)");
                    int y = ptrace(PTRACE_PEEKDATA, program, ORIG_RSI, NULL);
                    check_or_fail(y, "print (y)");
                    uint16_t *chars = ptrace(PTRACE_PEEKDATA, program, ORIG_RDX, NULL);
                    check_or_fail((int) chars, "print (chars)");
                    int n = ptrace(PTRACE_PEEKDATA, program, ORIG_R10, NULL);
                    check_or_fail((int) chars, "print (n)");
                    sys_print(x, y, chars, n);
                    refresh();
                    break;
                case SYSCALL_SETCURSOR:
                    int x = ptrace(PTRACE_PEEKDATA, program, ORIG_RDI, NULL);
                    check_or_fail(x, "print (x)");
                    int y = ptrace(PTRACE_PEEKDATA, program, ORIG_RSI, NULL);
                    check_or_fail(y, "print (y)");
                    sys_setcursor(x, y);
                    refresh();
                    break;
                default:
                    fprintf(stderr, "Invalid syscall %d\n", syscall_nr);
                    endwin();
                    return 127;
            }

            ptrace(PTRACE_SYSEMU, program, NULL, NULL);
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