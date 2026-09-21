#include <stdio.h>

int main(void)
{
	unsigned long n = 1000000, i = 0, sum = 0;
    printf("Launch sequence initiated!\n");
    while(i < n)
    {
        sum += i;
        i++;
        //Uncomment this while debugging
        // if(i % 1000 == 0){
        //     printf("We're at %lu right now...\n",i);
        // }
    }
    printf("It's the finish line! The sum is %lu\n", sum);
	return 0;
}