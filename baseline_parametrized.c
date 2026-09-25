#include <stdio.h>
#include <stdlib.h>

/*
 * Parametrized baseline worker for popen().
 *
 * It calculates the sum from lower to upper - 1.
 *
 * Usage:
 * ./baseline_parametrized <lower> <upper>
 */
int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(
            stderr,
            "Usage: %s <lower> <upper>\n",
            argv[0]
        );

        return 1;
    }

    /* Convert command-line arguments from text to numbers */
    unsigned long lower =
        strtoul(argv[1], NULL, 10);

    unsigned long upper =
        strtoul(argv[2], NULL, 10);

    if (upper < lower)
    {
        fprintf(
            stderr,
            "Upper limit must not be smaller than lower limit\n"
        );

        return 1;
    }

    /*
     * Use 128 bits because the sum for N = 10^10
     * exceeds the capacity of a 64-bit integer.
     */
    unsigned __int128 partial_sum = 0;

    /*
     * The lower limit is inclusive.
     * The upper limit is exclusive.
     */
    for (unsigned long i = lower; i < upper; i++)
    {
        partial_sum += i;
    }

    /*
     * fscanf() cannot directly read an unsigned
     * 128-bit integer. Split it into two 64-bit parts.
     */
    unsigned long long high_part =
        (unsigned long long)(partial_sum >> 64);

    unsigned long long low_part =
        (unsigned long long)partial_sum;

    /*
     * Do not print anything else to stdout.
     * The parent expects exactly these two numbers.
     */
    printf("%llu %llu\n", high_part, low_part);

    return 0;
}
