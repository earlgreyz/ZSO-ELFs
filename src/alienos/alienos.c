#include "alienos.h"
#include "../system/random.h"
#include "graphics.h"

#include <stdlib.h>
#include <curses.h>

// Extract symbol from an alien character
#define CHAR_COL(c) (short) (((c) & 0x0f00) >> 8)
// Extract color from an alien character
#define CHAR_SYM(c) (char) ((c) & 0x00ff)

// Execute successfully or exit immediately
#define MUST(x) if ((x) != OK) return -1;

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

int sys_getrand(uint32_t *value) {
    if (getrandom(value, sizeof(uint32_t), 0) != sizeof(uint32_t)) {
        return -1;
    }
    return 0;
}

int sys_getkey(void) {
    int key = get_key();
    switch (key) {
        case KEY_DOWN:
            return OSKEY_DOWN;
        case KEY_UP:
            return OSKEY_UP;
        case KEY_LEFT:
            return OSKEY_LEFT;
        case KEY_RIGHT:
            return OSKEY_RIGHT;
        default:
            return key;
    }
}

int sys_print(int x, int y, uint16_t *chars, int n) {
    MUST(save_cursor())
    MUST(move_cursor(x, y))

    for (uint16_t *c = chars; c < chars + n; c++) {
        MUST(print_character(CHAR_SYM(*c), (short) (0x50 + CHAR_COL(*c))));
    }

    MUST(restore_cursor())
    MUST(refresh_window())
    return 0;
}

int sys_setcursor(int x, int y) {
    MUST(move_cursor(x, y))
    MUST(refresh_window())
    return 0;
}
