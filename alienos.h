#ifndef ALIENOS_H
#define ALIENOS_H


#include <stdint.h>

// Available syscalls
#define SYSCALL_END        0x00
#define SYSCALL_GETRAND    0x01
#define SYSCALL_GETKEY     0x02
#define SYSCALL_PRINT      0x03
#define SYSCALL_SETCURSOR  0x04

// Key codes
#define OSKEY_UP             0x80
#define OSKEY_LEFT           0x81
#define OSKEY_DOWN           0x82
#define OSKEY_RIGHT          0x83

// .pt_params section address
#define PT_PARAMS    0x60031337

/// End program with given status.
/// Invoked via SYSCALL_END call.
/// \param status exit code.
void sys_end(int status);

/// Generates random integer value.
/// Invoked via SYSCALL_GETRAND call.
/// \return random value.
uint32_t sys_getrand(void);

/// Blocks program execution until a key is pressed.
/// Invoked via SYSCALL_GETKEY call.
/// \return pressed key code.
int sys_getkey(void);

/// Prints a list of characters to the screen.
/// Invoked via SYSCALL_PRINT call.
/// \param x starting x position.
/// \param y starting y position.
/// \param chars list of characters to be printed.
/// \param n length of the characters list.
void sys_print(int x, int y, uint16_t *chars, int n);

/// Moves the cursor to the position.
/// Invoked via SYSCALL_SETCURSOR call.
/// \param x new x position.
/// \param y new y position.
void sys_setcursor(int x, int y);


#endif // ALIENOS_H
