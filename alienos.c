#define _GNU_SOURCE
#include "alienos.h"

#include <stdlib.h>
#include <curses.h>
#include <unistd.h>
#include <sys/syscall.h>

// For some reason it's not found in the headers
#define SYS_getrandom 318

// Extract symbol from an alien character
#define CHAR_COL(c) ((c) & 0x0ff0 >> 8)
// Extract color from an alien character
#define CHAR_SYM(c) ((c) & 0x00ff)

void sys_end(int status) {
    endwin();
    fprintf(stderr, "sys_end called with status %d\n", status);
    exit(status);
}

uint32_t sys_getrand(void) {
    uint32_t value = 0;
    if (syscall(SYS_getrandom, &value, sizeof(value), NULL) == -1) {
        return (uint32_t)(-1);
    }
    return value;
}

int sys_getkey(void) {
    int c =  getch();
    switch (c) {
        case KEY_ENTER: return OSKEY_ENTER;
        case KEY_UP: return OSKEY_UP;
        case KEY_LEFT: return OSKEY_LEFT;
        case KEY_RIGHT: return OSKEY_RIGHT;
        case KEY_DOWN: return OSKEY_DOWN;
        default:
            break;
    }
    return c;
}

void sys_print(int x, int y, uint16_t *chars, int n) {
    int oldy, oldx;
    getyx(stdscr, oldy, oldx);
    move(y, x); // TODO: handle error
    for (uint16_t * c = chars; c < chars + n; c++) {
        addch(CHAR_SYM(*c) | COLOR_PAIR(CHAR_COL(*c))); // TODO: handle error
    }
    move(oldy, oldx);
}

void sys_setcursor(int x, int y) {
    move(y, x); // TODO: handle error
}