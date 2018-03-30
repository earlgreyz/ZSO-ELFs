#include "graphics.h"

#include <stdio.h>
#include <stdlib.h>

int no_echo(void) {
    int err;
    if ((err = system ("/bin/stty raw")) != 0) {
        return err;
    }
    return system("stty -echo");
}

int echo(void) {
    int err;
    if ((err = system ("/bin/stty cooked")) != 0) {
        return err;
    }
    return system("stty echo");
}

int move_cursor(int x, int y) {
    return printf("\033[%d;%dH", x, y);
}

int save_cursor(void) {
    return printf("\033[s");
}

int restore_cursor(void) {
    return printf("\033[u");
}

int clear_screen(void) {
    return printf("\033[2J");
}