#include<stdio.h>
#include<pthread.h>
#include<stdlib.h>
#include <time.h>
#include <math.h>

//1st run 4999999999950000000000 Time elapsed: 115.000018


//same helper as the baseline
void print_u128(unsigned __int128 x) {
    if (x == 0) {
        printf("0");
        return;
    }
    char buf[128];
    int idx = sizeof(buf) - 1;
    buf[idx] = '\0';
    while (x > 0) {
        idx--;
        buf[idx] = "0123456789"[x % 10];
        x /= 10;
    }
    printf("%s", &buf[idx]);
}

typedef struct {
    unsigned long start;
    unsigned long end;
    unsigned __int128 partial_sum;
} thread_arg_t;

void *worker(void *arg) {
thread_arg_t *targ = (thread_arg_t *) arg;
unsigned __int128 local_sum=0;
 for (unsigned long i = targ->start; i < targ->end; i++) {
        local_sum += i;
    }
    targ->partial_sum = local_sum;
    return NULL;
}

double businessLogic(unsigned long n, int NUM_THREADS) {
    struct timespec start, end;
    pthread_t threads[NUM_THREADS];
    thread_arg_t args[NUM_THREADS];
    unsigned long chunk = n / NUM_THREADS;
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i=0;i<NUM_THREADS;i++){
        args[i].start = i * chunk;
        args[i].end = (i == NUM_THREADS - 1) ? n : (i + 1) * chunk;
        args[i].partial_sum = 0;
        pthread_create(&threads[i], NULL, worker, &args[i]);
    }
    unsigned __int128 sum = 0;
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
        sum += args[i].partial_sum;
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    printf("Finished! The sum is ");
    print_u128(sum);
    printf("\n");
    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1000000000.0;
    printf("Time elapsed: %0.6f\n", elapsed);
    return elapsed;
}

int main (int argc, char *argv[]){
    unsigned long n = strtoul(argv[1], NULL, 10);
    int totalIter = atoi(argv[2]);
    int NUM_THREADS = atoi(argv[3]);
    double elapsedTimes[totalIter];
    for (int i = 0; i < totalIter; i++) {
        printf("(%d)", i + 1);
        elapsedTimes[i] = businessLogic(n, NUM_THREADS);
    }
    double mean = 0.0, standardDeviation = 0.0;
    for (int i = 0; i < totalIter; i++) {
        mean += elapsedTimes[i];
    }
    mean /= totalIter;

    for (int i = 0; i < totalIter; i++) {
        standardDeviation += pow(elapsedTimes[i] - mean, 2);
    }
    standardDeviation = sqrt(standardDeviation / totalIter);

    printf("Mean: %0.6f\n", mean);
    printf("Standard Deviation: %0.6f\n", standardDeviation);

    return 0;
}

