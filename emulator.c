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


#define INIT_CLR(color, r, g, b) \
    if (init_color(color, r, g, b) != OK) \
        return ERR;

/// Initializes ncurses colors for AlienOS.
/// \return initialization status (OK|ERR)
static int initialize_colors() {
    if (has_colors() == FALSE)
        return ERR;
    start_color();

    INIT_CLR(CLR_BLACK, 0, 0, 0)
    INIT_CLR(CLR_BLUE, 0, 0, 1000)
    INIT_CLR(CLR_GREEN, 0, 1000, 0)
    INIT_CLR(CLR_TURQUOISE, 250, 900, 800)
    INIT_CLR(CLR_RED, 1000, 0, 0)
    INIT_CLR(CLR_PINK, 1000, 100, 500)
    INIT_CLR(CLR_YELLOW, 1000, 1000, 0)
    INIT_CLR(CLR_LIT_GREY, 300, 300, 300)
    INIT_CLR(CLR_DRK_GREY, 600, 600, 600)
    INIT_CLR(CLR_LIT_BLUE, 500, 700, 1000)
    INIT_CLR(CLR_LIT_GREEN, 500, 1000, 500)
    INIT_CLR(CLR_LIT_TURQUOISE, 500, 900, 800)
    INIT_CLR(CLR_LIT_RED, 1000, 500, 500)
    INIT_CLR(CLR_LIT_PINK, 1000, 800, 800)
    INIT_CLR(CLR_LIT_YELLOW, 1000, 1000, 500)
    INIT_CLR(CLR_WHITE, 1000, 1000, 1000)


    for (short i = 0; i <= 0x0f; i++) {
        init_pair(i, i, CLR_BLACK); // TODO: handle err
    }

    refresh();
    return OK;
}

static void check_or_fail(int err, const char * message) {
    if (err != 0 && errno) {
        endwin();
        fprintf(stderr, message);
        exit(EXIT_FAILURE);
    }
}

static void emulate_syscall(pid_t pid) {
    int err;
    uint16_t chars[512];
    struct user_regs_struct regs;
    char x, y;

    err = ptrace(PTRACE_GETREGS, pid, NULL, &regs);

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
            for (int i = 0; i < regs.r10; i++) {
                err = ptrace(PTRACE_PEEKTEXT, pid, regs.rdx + sizeof(uint16_t) * i, NULL);
                check_or_fail(err, "print");
                *(chars + i) = (uint16_t) err;
            }

            sys_print(regs.rdi, regs.rsi, chars, regs.r10);
            refresh();
            break;
        case SYSCALL_SETCURSOR:
            sys_setcursor(regs.rdi, regs.rsi);
            refresh();
            break;
        default:
            endwin();
            fprintf(stderr, "Invalid syscall %d\n", regs.orig_rax);
            exit(127);
    }

    ptrace(PTRACE_SYSEMU, pid, NULL, NULL);
}

void run_emulator(pid_t pid) {
    int status, err;

    initscr();
    noecho();

    if (initialize_colors() != OK) {
        endwin();
        fprintf(stderr, "Unable to initialize colors\n");
        exit(EXIT_FAILURE);
    }

    wait(&status);
    ptrace(PTRACE_SYSCALL, pid, NULL, NULL);

    do {
        pid = wait(&status);
        if (pid == -1 || WIFEXITED(status)) {
            endwin();
            fprintf(stderr, "Wait failed\n");
            exit(EXIT_SUCCESS);
        }

        if (WSTOPSIG(status) == SIGTRAP) {
            emulate_syscall(pid);
        }
    } while (1);
}

void run_program(int argc, char *argv[]) {
    prctl(PR_SET_PDEATHSIG, SIGHUP);
    ptrace(PTRACE_TRACEME, 0, NULL, NULL);
    if (execv(argv[0], argv) == -1) {
        fprintf(stderr, "Unable to exec %s\n", argv[0]);
        exit(EXIT_FAILURE);
    }
}
