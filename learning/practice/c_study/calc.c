#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int add(int a, int b)
{
    return a + b;
}

int sub(int a, int b)
{
    return a - b;
}

int mul(int a, int b)
{
    return a * b;
}
int divide(int a, int b, int *result)
{
    if (b == 0) {
        return -1;
    }
    *result = a / b;
    return 0;
}
int main(int argc, char *argv[])
{
    if(argc != 4)
    {
        printf("error\n");
        return -1;
    }
    char *end_a;
    char *end_b;
    int a = strtol(argv[1], &end_a, 10);
    if (end_a == argv[1] || *end_a != '\0' || errno == ERANGE)
    {
        printf("input a invalid\n");
        return -1;
    }
    
    int b = strtol(argv[3], &end_b, 10);
    if (end_b == argv[3] || *end_b != '\0' || errno == ERANGE)
    {
        printf("input b invalid\n");
        return -1;
    }
    if((a > 1000 || a < -1000) || (b > 1000 || b < -1000))
    {
        printf("a or b invalid\n");
        return -1;
    }
    if (argv[2][1] != '\0') 
    {
        printf("argv[2] invalid: %s\n", argv[2]);
        return -1;
    }
    int result = 0;
    switch (*argv[2])
    {
    case '+':
        result = add(a, b);
        break;
    case '-':
        result = sub(a, b);
        break;
    case 'x':
        result = mul(a, b);
        break;
    case '/':
        if(divide(a, b, &result) != 0)
        {
            printf("*char[3] error\n");
            return -1;
        }
        break;
    default:
        printf("*char[2] error\n");
        return -1;
        break;
    }
    printf("%d\n", result);
    return 0;
}