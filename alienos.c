#include "alienos.h"
#include <stdlib.h>
#include <curses.h>
#include <sys/random.h>

// Extract symbol from an alien character
#define CHAR_SYM(c) ((c) & 0x00ff)
// Extract color from an alien character
#define CHAR_COL(c) ((c) & 0x0f00 >> 4)

void end(int status) {
    exit(status);
}

uint32_t getrand(void) {
    uint32_t value = 0;
    if (getrandom(&value, sizeof(value), NULL) == -1) {
        // This is the best we can do to signalise error.
        return (uint32_t)(-1);
    }
    return value;
}

int getkey(void) {
    return getch();
}

void print(int x, int y, uint16_t *chars, int n) {
    move(y, x); // TODO: handle error
    for (uint16_t * c = chars; c < chars + n; c++) {
        addch((const chtype)CHAR_SYM(*c) | COLOR_PAIR(CHAR_COL(*c))); // TODO: handle error
    }
}

void setcursor(int x, int y) {
    move(y, x); // TODO: handle error
}