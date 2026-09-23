#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#include <math.h>

// The program re-runs itself from the current directory
#define EXE_PATH "./multitask_popen"

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
    // Worker process
    // the parent sets SUM_START and SUM_END variables in the environment
    // and prints the 128-bit result as two 64-bit halves (hi lo) on stdout, which the parent reads through the popen() pipe.

    const char *env_start = getenv("SUM_START");
    const char *env_end = getenv("SUM_END");
    if (env_start != NULL && env_end != NULL)
    {
        unsigned long start_i = strtoul(env_start, NULL, 10);
        unsigned long end_i = strtoul(env_end, NULL, 10);
        unsigned __int128 local_sum = 0;

        for (unsigned long i = start_i; i < end_i; i++)
        {
            local_sum += i;
        }

        unsigned long long hi = (unsigned long long)(local_sum >> 64);
        unsigned long long lo = (unsigned long long)local_sum;
        printf("%llu %llu\n", hi, lo);
        return 0.0;
    }

    //Parent process ONLY
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
        snprintf(cmd, sizeof(cmd), "SUM_START=%lu SUM_END=%lu " EXE_PATH, start_i, end_i);

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
        int got;

        while (1)
        {
            errno = 0;
            got = fscanf(fps[p], "%llu %llu", &hi, &lo);
            if (got == 2)
                break;
            if (ferror(fps[p]) && errno == EINTR) // interrupted by a signal, so try again
            {
                //We wanted to see what signal was interrupting the read and terminating the process, 
                //same situation as fork implementation
                //fprintf(stderr, "worker %d: read interrupted (EINTR), retrying\n", p); 
                clearerr(fps[p]);
                continue;
            }
            break;
        }

        int saw_eof = feof(fps[p]);
        int saw_err = ferror(fps[p]);
        int saved_errno = errno;
        int status = pclose(fps[p]); // waits for the worker and closes the pipe

        //Print statements only for debugging
        // if (got != 2)
        // {
        //     any_failed = 1;
        //     fprintf(stderr, "worker %d: fscanf returned %d (eof=%d, error=%d, errno=%d: %s)\n",
        //             p, got, saw_eof, saw_err, saved_errno, strerror(saved_errno));
        //     if (WIFEXITED(status))
        //         fprintf(stderr, "  worker exited with code %d\n", WEXITSTATUS(status));
        //     else if (WIFSIGNALED(status))
        //         fprintf(stderr, "  worker killed by signal %d\n", WTERMSIG(status));
        //     continue;
        // }

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