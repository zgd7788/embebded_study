#include "task13.c"
#include <stdio.h>
int main()
{
    uint8_t storage[3];
    ring_buf_t rb;
    uint8_t value;

    /* 1. 初始化环形缓冲区 */
    if (rb_init(&rb, storage, 3) != 0)
    {
        printf("init failed\n");
        return -1;
    }

    /* 2. 依次存入 A、B、C */
    rb_push(&rb, 'A');
    rb_push(&rb, 'B');
    rb_push(&rb, 'C');

    /* 3. 测试满队列：继续存入应该失败 */
    printf("push D when full: %d\n", rb_push(&rb, 'D'));

    /* 4. 取出 A、B */
    rb_pop(&rb, &value);
    printf("pop: %c\n", value);

    rb_pop(&rb, &value);
    printf("pop: %c\n", value);

    /* 5. 再存入 D、E */
    rb_push(&rb, 'D');
    rb_push(&rb, 'E');

    /* 6. 连续读取剩余数据 */
    while (rb_pop(&rb, &value) == 0)
    {
        printf("pop: %c\n", value);
    }

    return 0;
}
