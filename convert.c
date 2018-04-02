#include "convert.h"

#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>

int str_to_int(const char *str, int *val) {
    char *end;
    errno = 0;
    const long lval = strtol(str, &end, 10);

    if (end == str) {
        // Not a decimal number
        return -1;
    } else if ('\0' != *end) {
        // Extra characters at the end of the input
        return -1;
    } else if ((LONG_MIN == lval || LONG_MAX == lval) && ERANGE == errno) {
        // Out of range for type long
        return -1;
    } else if (lval > INT_MAX || lval < INT_MIN) {
        // Out of range for type int
        return -1;
    }

    *val = (int) lval;
    return 0;
}