#include "emulate.h"
#include "alienos.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/ptrace.h>
#include <asm/ptrace.h>

#define IS_ERR(x) ((x) == -1)
#define IS_PTRACE_ERR(x) ((x) == -1 && errno)

void emulation_failure(struct alien_proc_struct *child, const char *message) {
    close(child->mem);
    end_alienos();
    fprintf(stderr, "%s\n", message);
    exit(EXIT_ALIENOS_FAIL);
}

void emulate_end(struct alien_proc_struct *child, struct user_regs_struct *regs) {
    close(child->mem);
    sys_end((int) regs->rdi);
}

void emulate_getrand(struct alien_proc_struct *child, struct user_regs_struct *regs) {
    uint32_t value;
    if (IS_ERR(sys_getrand(&value))) {
        emulation_failure(child, "sys_getrand");
    }
    regs->rax = value;

    if (IS_PTRACE_ERR(ptrace(PTRACE_SETREGS, child->pid, NULL, regs))) {
        emulation_failure(child, "PTRACE_SETREGS");
    }
}

void emulate_getkey(struct alien_proc_struct *child, struct user_regs_struct *regs) {
    regs->rax = (long long unsigned int) sys_getkey();
    if (IS_PTRACE_ERR(ptrace(PTRACE_SETREGS, child->pid, NULL, regs))) {
        emulation_failure(child, "PTRACE_SETREGS");
    }
}

void emulate_print(struct alien_proc_struct *child, struct user_regs_struct *regs) {
    size_t buffer_len = sizeof(uint16_t) * regs->r10;
    uint16_t *buffer = (uint16_t *) malloc(buffer_len);
    if (buffer == NULL) {
        emulation_failure(child, "malloc");
    }

    if (pread(child->mem, buffer, buffer_len, (off_t) regs->rdx) != (ssize_t) buffer_len) {
        free(buffer);
        emulation_failure(child, "pread");
    }

    if (IS_ERR(sys_print((int) regs->rdi, (int) regs->rsi, buffer, (int) regs->r10))) {
        free(buffer);
        emulation_failure(child, "sys_print");
    }
    free(buffer);
}

void emulate_setcursor(struct alien_proc_struct *child, struct user_regs_struct *regs) {
    if (IS_ERR(sys_setcursor((int) regs->rdi, (int) regs->rsi))) {
        emulation_failure(child, "sys_setcursor");
    }
}

void emulate_syscall(struct alien_proc_struct *child) {
    struct user_regs_struct regs;

    if (IS_PTRACE_ERR(ptrace(PTRACE_GETREGS, child->pid, NULL, &regs))) {
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