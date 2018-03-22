#ifndef ALIENOS_H
#define ALIENOS_H


#include <stdint.h>

// Available syscalls
#define SYSCALL_END        0x00
#define SYSCALL_GETRAND    0x01
#define SYSCALL_GETKEY     0x02
#define SYSCALL_PRINT      0x03
#define SYSCALL_SETCURSOR  0x04

// Arrow key codes
#define KEY_ENTER          0x0a
#define KEY_ASCII_FIRST    0x20
#define KEY_ASCII_LAST     0x7e
#define KEY_UP             0x80
#define KEY_LEFT           0x81
#define KEY_DOWN           0x82
#define KEY_RIGHT          0x83

// .pt_params section address
#define PT_PARAMS    0x60031337

// AlienOS colors
#define CLR_BLACK          0x00
#define CLR_BLUE           0x01
#define CLR_GREEN          0x02
#define CLR_TURQUOISE      0x03
#define CLR_RED            0x04
#define CLR_PINK           0x05
#define CLR_YELLOW         0x06
#define CLR_LIT_GREY       0x07
#define CLR_DRK_GREY       0x08
#define CLR_LIT_BLUE       0x09
#define CLR_LIT_GREEN      0x0a
#define CLR_LIT_TURQUOISE  0x0b
#define CLR_LIT_RED        0x0c
#define CLR_LIT_PINK       0x0d
#define CLR_LIT_YELLOW     0x0e
#define CLR_WHITE          0x0f

/// End program with given status.
/// Invoked via SYSCALL_END call.
/// \param status exit code.
void end(int status);

/// Generates random integer value.
/// Invoked via SYSCALL_GETRAND call.
/// \return random value.
uint32_t getrand(void);

/// Blocks program execution until a key is pressed.
/// Invoked via SYSCALL_GETKEY call.
/// \return pressed key code.
int getkey(void);

/// Prints a list of characters to the screen.
/// Invoked via SYSCALL_PRINT call.
/// \param x starting x position.
/// \param y starting y position.
/// \param chars list of characters to be printed.
/// \param n length of the characters list.
void print(int x, int y, uint16_t *chars, int n);

/// Moves the cursor to the position.
/// Invoked via SYSCALL_SETCURSOR call.
/// \param x new x position.
/// \param y new y position.
void setcursor(int x, int y);


#endif // ALIENOS_H
