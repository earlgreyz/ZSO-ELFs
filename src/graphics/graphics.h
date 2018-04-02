#ifndef ZSO_ELFS_GRAPHICS_H
#define ZSO_ELFS_GRAPHICS_H

#define CLR_BLACK          0x50
#define CLR_BLUE           0x51
#define CLR_GREEN          0x52
#define CLR_TURQUOISE      0x53
#define CLR_RED            0x54
#define CLR_PINK           0x55
#define CLR_YELLOW         0x56
#define CLR_LIT_GREY       0x57
#define CLR_DRK_GREY       0x58
#define CLR_LIT_BLUE       0x59
#define CLR_LIT_GREEN      0x5a
#define CLR_LIT_TURQUOISE  0x5b
#define CLR_LIT_RED        0x5c
#define CLR_LIT_PINK       0x5d
#define CLR_LIT_YELLOW     0x5e
#define CLR_WHITE          0x5f

/// Starts graphics mode.
/// \return (OK|ERR)
int start_window(void);

/// Ends graphics mode.
/// \return (OK|ERR)
int end_window(void);

/// Flushes output.
/// \return (OK|ERR)
int refresh_window(void);

/// Moves cursor to given position.
/// \param x space from x.
/// \param y space from y.
/// \return (OK|ERR)
int move_cursor(int x, int y);

/// Saves cursor current position.
/// \return (OK|ERR)
int save_cursor(void);

/// Restores saved cursor position.
/// \return (OK|ERR)
int restore_cursor(void);

/// Prints the given character at the current cursor position.
/// \param character character to print.
/// \param color character color.
/// \return (OK|ERR)
int print_character(char character, short color);

/// Blocks program execution until a key is pressed.
/// \return key code.
int get_key(void);

#endif // ZSO_ELFS_GRAPHICS_H
