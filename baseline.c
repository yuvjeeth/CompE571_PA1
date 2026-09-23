#include <stdio.h>
#include <time.h>
#include <math.h>

// timings for (n):1e8, 1e9, 1e10, 20 iterations for each, avg and std dev for each n


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

double businessLogic(unsigned long n)
{
    struct timespec start, end;
    unsigned __int128 sum = 0; // This is because the 64bit variable overflows, so 128 bit
    unsigned long i = 0;
    printf("Sequence initiated!\n");
    clock_gettime(CLOCK_MONOTONIC, &start);
    while (i < n)
    {
        sum += i;
        i++;
        // Uncomment this while debugging DO NOT UNCOMMENT WHEN TAKING RESULTS
        //  if(i % 1e4 == 0){
        //  printf("We're at %lu right now...\n",i);
        //  }
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    printf("Finished! The sum is ");
    print_u128(sum);
    printf("\n");
    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1000000000.0;
    printf("Time elapsed: %0.6f\n", elapsed);
    return elapsed;
}

int main(void)
{
    unsigned long n = 1e8;
    int i = 0, totalIter = 20;
    double elapsedTimes[totalIter];
    for(i = 0; i < totalIter; i++){
        printf("(%d)", i + 1);
        elapsedTimes[i] = businessLogic(n);
    }

    double mean = 0.0, standardDeviation = 0.0;
    for(int iElapsedTimes = 0; iElapsedTimes < totalIter; iElapsedTimes++){
        mean += elapsedTimes[iElapsedTimes];
    }
    mean /= totalIter;

    for(int iElapsedTimes = 0; iElapsedTimes < totalIter; iElapsedTimes++){
        standardDeviation += pow(elapsedTimes[iElapsedTimes] - mean, 2);
    }
    standardDeviation = sqrt(standardDeviation / totalIter);

    printf("Mean: %0.6f\n", mean);
    printf("Standard Deviation: %0.6f\n", standardDeviation);

    return 0;
}