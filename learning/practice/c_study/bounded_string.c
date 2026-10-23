#include <stdio.h>

#define SIZE_MAX 100
/*
    功能：把 src 复制到容量为 dst_size 的 dst 中，并保证结果是合法 C 字符串。
    参数：
        *dst:
        dst_size:
        *src:
    返回值：
*/
size_t bounded_copy(char *dst, size_t dst_size, const char *src)
{
    if(src == NULL || dst == NULL )
    {
        printf("parameter error\n");
        return SIZE_MAX;
    }
    if(dst_size == 0)
    {
        printf("dst_size can not be 0\n");
        return 0;
    }
    size_t i = 0;
    for (; i < dst_size - 1 && src[i] != '\0'; i++)
        dst[i] = src[i];

    dst[i] = '\0';                     
    return i; 
}

int main()
{
    char src[6] = "hello";
    char dest[32];

    bounded_copy(dest, 0, src);

    printf("%s\n", src);
    printf("%s\n", dest);
    printf("%d\n", (int)bounded_copy(dest, 0, src));
    return 0;
}