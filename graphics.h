#ifndef ZSO_ELFS_GRAPHICS_H
#define ZSO_ELFS_GRAPHICS_H

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

int start_window(void);

int end_window(void);

int refresh_window(void);

int move_cursor(int x, int y);

int save_cursor(void);

int restore_cursor(void);

int print_character(char character, short color);

int get_key(void);


#endif // ZSO_ELFS_GRAPHICS_H
