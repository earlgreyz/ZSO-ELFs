#include "alienos.h"
#include "random.h"
#include "graphics.h"

#include <stdlib.h>
#include <stdio.h>

// Extract symbol from an alien character
#define CHAR_COL(c) (((c) & 0x0f00) >> 8)
// Extract color from an alien character
#define CHAR_SYM(c) ((c) & 0x00ff)

static const char *colors[16] = {
        FMT(FOREGROUND_COL_BLACK),
        FMT(GEN_FORMAT_BRIGHT";"FOREGROUND_COL_BLUE),
        FMT(GEN_FORMAT_BRIGHT";"FOREGROUND_COL_GREEN),
        FMT(GEN_FORMAT_BRIGHT";"FOREGROUND_COL_CYAN),
        FMT(GEN_FORMAT_BRIGHT";"FOREGROUND_COL_RED),
        FMT(GEN_FORMAT_BRIGHT";"FOREGROUND_COL_MAGENTA),
        FMT(GEN_FORMAT_BRIGHT";"FOREGROUND_COL_YELLOW),
        FMT(GEN_FORMAT_DIM";"FOREGROUND_COL_WHITE),
        FMT(GEN_FORMAT_BRIGHT";"FOREGROUND_COL_BLACK),
        FMT(GEN_FORMAT_DIM";"FOREGROUND_COL_BLUE),
        FMT(GEN_FORMAT_DIM";"FOREGROUND_COL_GREEN),
        FMT(GEN_FORMAT_DIM";"FOREGROUND_COL_CYAN),
        FMT(GEN_FORMAT_DIM";"FOREGROUND_COL_RED),
        FMT(GEN_FORMAT_DIM";"FOREGROUND_COL_MAGENTA),
        FMT(GEN_FORMAT_DIM";"FOREGROUND_COL_YELLOW),
        FMT(FOREGROUND_COL_WHITE)
};

void sys_end(int status) {
    exit(status);
}

uint32_t sys_getrand(void) {
    uint32_t value = 0;
    if (getrandom(&value, sizeof(value), 0) == -1) {
        return (uint32_t)(-1);
    }
    return value;
}

int sys_getkey(void) {
    int key = getchar();
    if (key == 27) {
        key = getchar();
        if (key != 91) {
            return key;
        }
        key = getchar();
        switch (key) {
            case 'A': return OSKEY_UP;
            case 'B': return OSKEY_DOWN;
            case 'C': return OSKEY_RIGHT;
            case 'D': return OSKEY_LEFT;
            default: return key;
        }
    }
    return key;
}

void sys_print(int x, int y, uint16_t *chars, int n) {
    save_cursor();
    move_cursor(x, y);
    for (uint16_t * c = chars; c < chars + n; c++) {
        fputs(colors[CHAR_COL(*c)], stdout);
        printf("%c", CHAR_SYM(*c));
        fputs(NO_FMT, stdout);
    }
    restore_cursor();
}

void sys_setcursor(int x, int y) {
    move_cursor(x, y);
}