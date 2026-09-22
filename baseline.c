#include<stdio.h>
#include<stdlib.h>
#include<time.h>
void print_int128(__int128_t number)//holding larger integer of sum in different function
{
    if(number>=10)
    {
        print_int128(number/10);
    }
    printf("%d", (int)(number%10));
}
int main(int argc,char *argv[])// argc take two argument
{// argv take character one base line one input
    if(argc !=2)// it will check the condition if its correct it will proceed further
    {
        printf("Please provideN\n");
        return 1;

    }
    __int128_t sum=0;// assing sum =0;
    struct  timespec start;
    struct timespec end;
    clock_gettime(CLOCK_MONOTONIC,&start);
    double elapsed;
    long long N=atoll(argv[1]);// atoll will change character to numbers 
     for(long long i=0;i<N;i++)
     {
        sum +=i;
     }
      clock_gettime(CLOCK_MONOTONIC,&end);
    // printf("N =%lld\n", N);
     //printf("sum=%lld\n", sum);// it will print sum
     printf("Expected+=");
     print_int128(sum);
     printf("\n");

     elapsed=(end.tv_sec-start.tv_sec)+(end.tv_nsec-start.tv_nsec)/1000000000.0;
     printf("Time=%.6f seconds\n", elapsed);
    return 0;
}
