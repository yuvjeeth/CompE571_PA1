#include <stdio.h>
#include <time.h>

//timings for (n):1e8, 1e9, 1e10, 50 iterations for each, avg and std dev for each n
//quick results for 1 iteration
//1e8 - 0.475023 (MacBook Neo - A18Pro - 8GB RAM)
//1e9 - 4.773511 (MacBook Neo - A18Pro - 8GB RAM)
//1e10 - 48.162713 (MacBook Neo - A18Pro - 8GB RAM)


//We're using this helper function to convert int128 type to string
//In order to print it to console
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

int main(void)
{
    struct timespec start, end;
    unsigned __int128 sum = 0; //This is because the 64bit variable overflows, so 128 bit
	unsigned long n = 1e8, i = 0;
    printf("Launch sequence initiated!\n");
    clock_gettime(CLOCK_MONOTONIC, &start);
    while(i < n)
    {
        sum += i;
        i++;
        //Uncomment this while debugging DO NOT UNCOMMENT WHEN TAKING RESULTS
        // if(i % 1e4 == 0){
        // printf("We're at %lu right now...\n",i);
        // }
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    printf("It's the finish line! The sum is ");
    print_u128(sum);
    printf("\n");
    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec)/1000000000.0;
    printf("Time elapsed: %0.6f\n",elapsed);
	return 0;
}