#include "graphics.h"
#include <curses.h>

#define CHECK_ERR(x) \
    if ((err = (x)) != OK) \
        return err;

#define INIT_CLR(color, r, g, b) \
    if (init_color(color, r, g, b) != OK) \
        return CHECK_ERR;

static int saved_x, saved_y;

static int start_colors(void) {
    int err;

    if (has_colors() == FALSE) {
        return ERR;
    }

    CHECK_ERR(init_color(CLR_BLACK, 0, 0, 0))
    CHECK_ERR(init_color(CLR_BLUE, 0, 0, 1000))
    CHECK_ERR(init_color(CLR_GREEN, 0, 1000, 0))
    CHECK_ERR(init_color(CLR_TURQUOISE, 250, 900, 800))
    CHECK_ERR(init_color(CLR_RED, 1000, 0, 0))
    CHECK_ERR(init_color(CLR_PINK, 1000, 100, 500))
    CHECK_ERR(init_color(CLR_YELLOW, 1000, 1000, 0))
    CHECK_ERR(init_color(CLR_LIT_GREY, 300, 300, 300))
    CHECK_ERR(init_color(CLR_DRK_GREY, 600, 600, 600))
    CHECK_ERR(init_color(CLR_LIT_BLUE, 500, 700, 1000))
    CHECK_ERR(init_color(CLR_LIT_GREEN, 500, 1000, 500))
    CHECK_ERR(init_color(CLR_LIT_TURQUOISE, 500, 900, 800))
    CHECK_ERR(init_color(CLR_LIT_RED, 1000, 500, 500))
    CHECK_ERR(init_color(CLR_LIT_PINK, 1000, 800, 800))
    CHECK_ERR(init_color(CLR_LIT_YELLOW, 1000, 1000, 500))
    CHECK_ERR(init_color(CLR_WHITE, 1000, 1000, 1000))

    for (short i = 0x00; i <= 0x0f; i++) {
        CHECK_ERR(init_pair(i, i, CLR_BLACK))
    }

    CHECK_ERR(refresh())
    return OK;
}

int start_window(void) {
    int err;
    CHECK_ERR(initscr())
    CHECK_ERR(start_colors())
    CHECK_ERR(keypad(stdscr, TRUE))
    CHECK_ERR(noecho())
    return OK;
}

int end_window(void) {
    int err;
    CHECK_ERR(echo())
    CHECK_ERR(endwin())
    return OK;
}

int refresh_window(void) {
    return refresh();
}

int move_cursor(int x, int y) {
    return move(y, x);
}

int save_cursor(void) {
    return getyx(stdscr, saved_y, saved_x);
}

int restore_cursor(void) {
    return move(saved_y, saved_x);
}

int print_character(char character, short color) {
    return addch(character| COLOR_PAIR(color));
}

int get_key(void) {
    return getch();
}