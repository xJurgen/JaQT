/*
*
*   Author: Jiří Veverka
*   Helper function to get float number as string
*
*/

#include "print_float.h"

#include <stdio.h>
#include <math.h>

int get_float(char *str, size_t len, float number)
{
    char *sign;
    int val1 = number, val2;
    if (number < 0) {
        *sign = "-";
    } else {
        *sign = "";
    }

    int val2 = trunc((float)(number - val1) * 10000);
    return snprintf(str, len, "%s%d.%04d", sign, val1, val2);
}
