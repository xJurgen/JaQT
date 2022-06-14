#include "print_float.h"

#include <stdio.h>
#include <math.h>

int format_float(float number, char *str, size_t lenstr)
{
    char *tmpSign = (number < 0) ? "-" : "";
    float tmpVal = (number < 0) ? -number : number;

    int tmpInt1 = tmpVal;                  // Get the integer (678).
    float tmpFrac = tmpVal - tmpInt1;      // Get fraction (0.0123).
    int tmpInt2 = trunc(tmpFrac * 10000);  // Turn into integer (123).

    return snprintf(str, lenstr, "%s%d.%04d", tmpSign, tmpInt1, tmpInt2);
}
