#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <math.h>

// timings for (n):1e8, 1e9, 1e10, 50 iterations for each, avg and std dev for each n

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
    unsigned __int128 sum = 0; // This is because the 64bit variable overflows, so 128 bit
    unsigned long chunkSize = n / NUM_TASKS;
    int readfds[NUM_TASKS];

    printf("Sequence initiated!\n");
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int p = 0; p < NUM_TASKS; p++)
    {
        int pfds[2]; //This is the replacement for the dup function
        if (pipe(pfds) == -1)
        {
            perror("pipe");
            exit(1);
        }

        pid_t pid = fork();
        if (pid == -1)
        {
            perror("fork");
            exit(1);
        }

        if (pid == 0)
        {
            // Child process
            close(pfds[0]); // close unused read end

            unsigned __int128 local_sum = 0;
            unsigned long start_i = p * chunkSize;
            unsigned long end_i = (p == NUM_TASKS - 1) ? n : (p + 1) * chunkSize;

            for (unsigned long i = start_i; i < end_i; i++)
            {
                local_sum += i;
            }

            //Here we write the partial sum to the pipe (so we don't want the dup function, this does that)
            if (write(pfds[1], &local_sum, sizeof(local_sum)) != (ssize_t)sizeof(local_sum))
                _exit(1);
            close(pfds[1]);
            _exit(0); // child must never fall through to the parent code so exit it
        }

        // Parent process
        close(pfds[1]);       // close unused write end
        readfds[p] = pfds[0]; // save read end to collect result later in parent
    }

    // Read back partial sums from each child
    for (int p = 0; p < NUM_TASKS; p++)
    {
        unsigned __int128 partial_sum = 0;
        ssize_t got = 0;
        char *dst = (char *)&partial_sum;

        // read() can return fewer bytes than asked, so loop until we have all 16
        while (got < (ssize_t)sizeof(partial_sum))
        {
            ssize_t r = read(readfds[p], dst + got, sizeof(partial_sum) - got);
            if (r < 0 && errno == EINTR)
                continue; // interrupted by a signal, not a real error so don't stop again only happened on the MacOS and not Ubuntu
            if (r <= 0)
                break;
            got += r;
        }
        close(readfds[p]);

        sum += partial_sum;

        // wait() can also be interrupted by a signal, which only happened on our MacOS systems and not Ubuntu
        while (wait(NULL) == -1 && errno == EINTR)
            ;
    }

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
        printf("(%d)", i + 1);
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