#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <math.h>

#define EXE_PATH "./baseline_parametrized"

// We're using this helper function to convert int128 type to string
// In order to print it to console
void print_u128(unsigned __int128 x)
{
    if (x == 0)
    {
        printf("0");
        return;
    }

    char buf[128];
    int idx = sizeof(buf) - 1;
    buf[idx] = '\0';

    while (x > 0)
    {
        idx--;
        buf[idx] = "0123456789"[x % 10];
        x /= 10;
    }

    printf("%s", &buf[idx]);
}

double businessLogic(unsigned long n, int NUM_TASKS)
{
    struct timespec start, end;
    unsigned __int128 sum = 0;
    unsigned long chunkSize = n / NUM_TASKS;
    FILE *fps[NUM_TASKS];

    printf("Sequence initiated!\n");
    clock_gettime(CLOCK_MONOTONIC, &start);

    // Launch all workers first so they run concurrently
    for (int p = 0; p < NUM_TASKS; p++)
    {
        unsigned long start_i = p * chunkSize;
        unsigned long end_i = (p == NUM_TASKS - 1) ? n : (p + 1) * chunkSize;

        char cmd[256];
        snprintf(cmd, sizeof(cmd), EXE_PATH " %lu %lu", start_i, end_i);

        //Print statements only for debugging
        //fprintf(stderr, "launching worker %d: %s\n", p, cmd);

        fps[p] = popen(cmd, "r");
        if (fps[p] == NULL)
        {
            perror("popen");
            exit(1);
        }
    }

    // Read back partial sums from each worker
    int any_failed = 0;
    for (int p = 0; p < NUM_TASKS; p++)
    {
        unsigned long long hi = 0, lo = 0;
        int got = fscanf(fps[p], "%llu %llu", &hi, &lo);
        int status = pclose(fps[p]); // waits for the worker and closes the pipe

        if (got != 2 || status == -1 || !WIFEXITED(status) || WEXITSTATUS(status) != 0)
        {
            any_failed = 1;
            fprintf(stderr, "worker %d failed\n", p);
            continue;
        }

        sum += ((unsigned __int128)hi << 64) | lo;
    }
    if (any_failed)
        exit(1);

    clock_gettime(CLOCK_MONOTONIC, &end);
    printf("Finished! The sum is ");
    print_u128(sum);
    printf("\n");
    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1000000000.0;
    printf("Time elapsed: %0.6f\n", elapsed);
    return elapsed;
}

int main(int argc, char *argv[])
{
    unsigned long n = strtoul(argv[1], NULL, 10);
    int totalIter = atoi(argv[2]);
    int NUM_TASKS = atoi(argv[3]);

    double elapsedTimes[totalIter];

    for (int i = 0; i < totalIter; i++)
    {
        //printf("(%d)", i + 1);
        elapsedTimes[i] = businessLogic(n, NUM_TASKS);
    }

    double mean = 0.0, standardDeviation = 0.0;
    for (int i = 0; i < totalIter; i++)
    {
        mean += elapsedTimes[i];
    }
    mean /= totalIter;

    for (int i = 0; i < totalIter; i++)
    {
        standardDeviation += pow(elapsedTimes[i] - mean, 2);
    }
    standardDeviation = sqrt(standardDeviation / totalIter);

    printf("Mean: %0.6f\n", mean);
    printf("Standard Deviation: %0.6f\n", standardDeviation);

    return 0;
}