#define _GNU_SOURCE
#include "alienos.h"

#include <stdlib.h>
#include <curses.h>
#include <unistd.h>
#include <sys/syscall.h>

// For some reason it's not found in the unistd.h
#define SYS_getrandom 318

// Extract symbol from an alien character
#define CHAR_COL(c) ((c) & 0x0ff0 >> 8)
// Extract color from an alien character
#define CHAR_SYM(c) ((c) & 0x00ff)

// Init color or return from function with ERR
#define INIT_CLR(color, r, g, b) \
    if (init_color(color, r, g, b) != OK) \
        return ERR;

static int initialize_colors() {
    if (has_colors() == FALSE) {
        return ERR;
    }
    start_color();

    INIT_CLR(CLR_BLACK, 0, 0, 0)
    INIT_CLR(CLR_BLUE, 0, 0, 1000)
    INIT_CLR(CLR_GREEN, 0, 1000, 0)
    INIT_CLR(CLR_TURQUOISE, 250, 900, 800)
    INIT_CLR(CLR_RED, 1000, 0, 0)
    INIT_CLR(CLR_PINK, 1000, 100, 500)
    INIT_CLR(CLR_YELLOW, 1000, 1000, 0)
    INIT_CLR(CLR_LIT_GREY, 300, 300, 300)
    INIT_CLR(CLR_DRK_GREY, 600, 600, 600)
    INIT_CLR(CLR_LIT_BLUE, 500, 700, 1000)
    INIT_CLR(CLR_LIT_GREEN, 500, 1000, 500)
    INIT_CLR(CLR_LIT_TURQUOISE, 500, 900, 800)
    INIT_CLR(CLR_LIT_RED, 1000, 500, 500)
    INIT_CLR(CLR_LIT_PINK, 1000, 800, 800)
    INIT_CLR(CLR_LIT_YELLOW, 1000, 1000, 500)
    INIT_CLR(CLR_WHITE, 1000, 1000, 1000)

    for (short i = 0; i <= 0x0f; i++) {
        init_pair(i, i, CLR_BLACK); // TODO: handle err
    }

    refresh();
    return OK;
}

int start_window(void) {
    // TODO: Check return codes.
    initscr();
    keypad(stdscr, TRUE);
    noecho();
    return initialize_colors();
}

int end_window(void) {
    // TODO: Check return codes.
    endwin();
    reset_shell_mode();
    return OK;
}

void sys_end(int status) {
    end_window();
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
    int key =  getch();
    switch (key) {
        case KEY_ENTER: return OSKEY_ENTER;
        case KEY_UP: return OSKEY_UP;
        case KEY_LEFT: return OSKEY_LEFT;
        case KEY_RIGHT: return OSKEY_RIGHT;
        case KEY_DOWN: return OSKEY_DOWN;
        default: return key;
    }
}

void sys_print(int x, int y, uint16_t *chars, int n) {
    int oldy, oldx;
    getyx(stdscr, oldy, oldx);

    move(y, x); // TODO: handle error
    for (uint16_t * c = chars; c < chars + n; c++) {
        const chtype symbol = (chtype) CHAR_SYM(*c) | (chtype) COLOR_PAIR(CHAR_COL(*c));
        addch(symbol); // TODO: handle error
    }

    move(oldy, oldx);
}

void sys_setcursor(int x, int y) {
    move(y, x); // TODO: handle error
}