#include <stdio.h>

int main(void)
{
	long n = 10000000000, i = 0, j = 0, numPrime = 0;
    printf("Elon Musk ki jai\n");
    while(i <= n)
    {
        for(j = 2; j < i; j++)
        {
            if(j % i == 0){
                continue;
                printf("%ld is not a prime number.\n", i);
            }
            else{
                numPrime++;
                printf("------%ld is a prime number------\n", i);
            }
        }
    }
	return 0;
}