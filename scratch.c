#include <stdio.h>
#include <stdint.h>
#include <wait.h>
#include <time.h>


int main()
{
    #define TEST 1
    int biggest = sizeof(int) * 8-1;
    printf("%d\n", TEST);
    #define TEST 2
    long int sign = 1<<biggest;
    printf("%d\n", TEST);
    for (int c = 0; c < 8; ++c) {
       printf("%d\n", c);
      }
    return 0;
}