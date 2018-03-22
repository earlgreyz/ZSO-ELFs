#include "emulator.h"

#include <curses.h>
#include "alienos.h"

int initialize_emulator() {
    if (has_colors() == FALSE) {
        return 1;
    }
    start_color();

    if (init_color(CLR_BLACK, 0, 0, 0) != 0) return 1;
    if (init_color(CLR_BLUE, 0, 0, 1000) != 0) return 1;
    if (init_color(CLR_GREEN, 0, 1000, 0) != 0) return 1;
    if (init_color(CLR_TURQUOISE, 250, 900, 800) != 0) return 1;
    if (init_color(CLR_RED, 1000, 0, 0) != 0) return 1;
    if (init_color(CLR_PINK, 1000, 100, 500) != 0) return 1;
    if (init_color(CLR_YELLOW, 1000, 1000, 0) != 0) return 1;
    if (init_color(CLR_LIT_GREY, 300, 300, 300) != 0) return 1;
    if (init_color(CLR_DRK_GREY, 600, 600, 600) != 0) return 1;
    if (init_color(CLR_LIT_BLUE, 500, 700, 1000) != 0) return 1;
    if (init_color(CLR_LIT_GREEN, 500, 1000, 500) != 0) return 1;
    if (init_color(CLR_LIT_TURQUOISE, 500, 900, 800) != 0) return 1;
    if (init_color(CLR_LIT_RED, 1000, 500, 500) != 0) return 1;
    if (init_color(CLR_LIT_PINK, 1000, 800, 800) != 0) return 1;
    if (init_color(CLR_LIT_YELLOW, 1000, 1000, 500) != 0) return 1;
    if (init_color(CLR_WHITE, 1000, 1000, 1000) != 0) return 1;

    for (short i = 0; i <= 0x0f; i++) {
        if (init_pair(i, i, CLR_BLACK) != 0) return 1;
    }

    return 0;
}
