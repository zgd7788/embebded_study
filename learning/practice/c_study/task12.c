#include <stdlib.h>
#include <stdio.h>
/*
    功能：动态分配一个 int 对象，将其初始化为 value
    参数：
        @value：可以是任意 int 值
    返回值：成功返回指向新对象的指针，解引用后等于 value 
              失败返回分配失败时返回 NULL，不打印错误 
*/
int *int_create(int value)
{
    int *p = malloc(sizeof(int));
    if(p == NULL)
    {
        return NULL;
    }
    *p = value;
    return p;
}

int main()
{
    int *p = malloc(sizeof(int));
    if(p == NULL)
    {
        fprintf(stderr, "malloc failed\n");
        return 1;
    }
    p = int_create(5);
    printf("%d\n", *p);
    free(p);                  
    p = NULL;            

    return 0;
}