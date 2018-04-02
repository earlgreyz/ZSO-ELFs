#ifndef ZSO_ELFS_EMULATE_H
#define ZSO_ELFS_EMULATE_H

#define _GNU_SOURCE
#include <unistd.h>

#include <sys/reg.h>
#include <sys/user.h>

/// AlienOS process information for the tracer.
struct alien_proc_struct {
    pid_t pid;
    int mem;
};

/// Emulates sys_end syscall.
/// \param child alien process tracer data.
/// \param regs alien process registers data.
void emulate_end(struct alien_proc_struct *child, struct user_regs_struct *regs);

/// Emulates sys_getrand syscall.
/// \param child alien process tracer data.
/// \param regs alien process registers data.
void emulate_getrand(struct alien_proc_struct *child, struct user_regs_struct *regs);

/// Emulates sys_getkey syscall.
/// \param child alien process tracer data.
/// \param regs alien process registers data.
void emulate_getkey(struct alien_proc_struct *child, struct user_regs_struct *regs);

/// Emulates sys_print syscall.
/// \param child alien process tracer data.
/// \param regs alien process registers data.
void emulate_print(struct alien_proc_struct *child, struct user_regs_struct *regs);

/// Emulates sys_setcursor syscall.
/// \param child alien process tracer data.
/// \param regs alien process registers data.
void emulate_setcursor(struct alien_proc_struct *child, struct user_regs_struct *regs);

/// Emulates AlienOS process syscall.
/// \param child alien process tracer data.
void emulate_syscall(struct alien_proc_struct *child);

#endif //ZSO_ELFS_EMULATE_H
