#include "alienos.h"
#include "random.h"
#include "graphics.h"

#include <stdlib.h>
#include <curses.h>

// Extract symbol from an alien character
#define CHAR_COL(c) (((c) & 0x0f00) >> 8)
// Extract color from an alien character
#define CHAR_SYM(c) ((c) & 0x00ff)

// Execute without error or exit with error
#define MUST(x) if ((x) != 0) sys_end(EXIT_FAILURE);

int start_alienos(void) {
    return start_window();
}

int end_alienos(void) {
    return end_window();
}

void sys_end(int status) {
    (void) end_window();
    exit(status);
}

uint32_t sys_getrand(void) {
    uint32_t value = 0;
    MUST(getrandom(&value, sizeof(value), 0))
    return value;
}

int sys_getkey(void) {
    int key = get_key();
    switch (key) {
        case KEY_DOWN: return OSKEY_DOWN;
        case KEY_UP: return OSKEY_UP;
        case KEY_LEFT: return OSKEY_LEFT;
        case KEY_RIGHT: return OSKEY_RIGHT;
        default: return key;
    }
}

void sys_print(int x, int y, uint16_t *chars, int n) {
    MUST(save_cursor())
    MUST(move_cursor(x, y))

    for (uint16_t * c = chars; c < chars + n; c++) {
        MUST(print_character(CHAR_SYM(*c), CHAR_COL(*c)))
    }

    MUST(restore_cursor())
    MUST(refresh_window())
}

void sys_setcursor(int x, int y) {
    MUST(move_cursor(x, y))
    MUST(refresh_window())
}
