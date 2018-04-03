#include "graphics.h"
#include <curses.h>

#define MUST(x) if ((err = (x)) != OK) return err;

static int saved_x, saved_y;

static int start_colors(void) {
    int err;

    if (has_colors() == FALSE) {
        return ERR;
    }

    start_color();

    MUST(init_color(CLR_BLACK, 0, 0, 0))
    MUST(init_color(CLR_BLUE, 0, 0, 1000))
    MUST(init_color(CLR_GREEN, 0, 1000, 0))
    MUST(init_color(CLR_TURQUOISE, 250, 900, 800))
    MUST(init_color(CLR_RED, 1000, 0, 0))
    MUST(init_color(CLR_PINK, 1000, 100, 500))
    MUST(init_color(CLR_YELLOW, 1000, 1000, 0))
    MUST(init_color(CLR_LIT_GREY, 300, 300, 300))
    MUST(init_color(CLR_DRK_GREY, 600, 600, 600))
    MUST(init_color(CLR_LIT_BLUE, 500, 700, 1000))
    MUST(init_color(CLR_LIT_GREEN, 500, 1000, 500))
    MUST(init_color(CLR_LIT_TURQUOISE, 500, 900, 800))
    MUST(init_color(CLR_LIT_RED, 1000, 500, 500))
    MUST(init_color(CLR_LIT_PINK, 1000, 800, 800))
    MUST(init_color(CLR_LIT_YELLOW, 1000, 1000, 500))
    MUST(init_color(CLR_WHITE, 1000, 1000, 1000))

    for (short i = 0x50; i <= 0x5f; i++) {
        MUST(init_pair(i, i, CLR_BLACK))
    }

    MUST(refresh())
    return OK;
}

int start_window(void) {
    int err;
    initscr();
    MUST(start_colors())
    MUST(keypad(stdscr, TRUE))
    MUST(noecho())
    return OK;
}

int end_window(void) {
    int err;
    MUST(echo())
    MUST(endwin())
    return OK;
}

int refresh_window(void) {
    return refresh();
}

int move_cursor(int x, int y) {
    return move(y, x);
}

int save_cursor(void) {
    getyx(stdscr, saved_y, saved_x);
    if (saved_y == -1 && saved_x == -1) {
        return ERR;
    }
    return OK;
}

int restore_cursor(void) {
    return move(saved_y, saved_x);
}

int print_character(char character, short color) {
    return addch((chtype) character | (chtype) COLOR_PAIR(color));
}

int get_key(void) {
    return getch();
}