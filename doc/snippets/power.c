long int pow(long int x, unsigned int n)
{
    long int a = x, p = 1;
    while (n > 0)
    {
        if ((n & 1) != 0)
            p *= a;
        a *= a;
        n >>= 1;
    }
    return p;
}

//Instead of itoa, you can use the sprintf function with a format specifier of %02d to force the inclusion of a leading zero:

#include <stdio.h>
#include <stdlib.h>

int main()
{
    int i = 7;
    char buffer[10];
//  itoa(buffer, i, 10);
    sprintf(buffer, "%02d", i);
    printf("decimal: %s\n", buffer);
    return 0;
}