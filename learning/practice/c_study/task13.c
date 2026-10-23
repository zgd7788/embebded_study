#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint8_t *data;       // 调用者提供的字节数组
    size_t capacity;    // 数组容量
    size_t read_pos;    // 下次读取的位置
    size_t write_pos;   // 下次写入的位置
    size_t count;       // 当前有效字节数
} ring_buf_t;

/*
    功能：绑定数组，把两个下标和 count 设为 0，无需清零数组
    参数：
        @rb:指向可写结构体
        @storage:指向至少 capacity 字节的可写数组
        @capacity:数组容量
    返回值：
        成功 0；任一指针为 NULL 或容量为 0，返回 -1
*/
int rb_init(ring_buf_t *rb, uint8_t *storage, size_t capacity)
{
    if((rb == NULL)||(storage == NULL)||(capacity == 0))
    {
        return -1;
    }
    rb->count = 0;
    rb->read_pos = 0;
    rb->write_pos = 0;
    rb->capacity = capacity;
    rb->data = storage;
    return 0;
}

/*
    功能：向已初始化的队列末尾存入数值，更新下标和数量
    参数：
        @rb:指向可写结构体
        @value：要存入的数值
    返回值：
        成功 0；rb == NULL 返回 -1；满返回 -2

*/
int rb_push(ring_buf_t *rb, uint8_t value)
{
    if(rb == NULL)
    {
        return -1;
    }
    if(rb->count == rb->capacity)
    {
        return -2;
    }
    if(rb->count < rb->capacity)
    {
        rb->data[rb->write_pos] = value;
        rb->count++;
        rb->write_pos = (rb->write_pos + 1) % rb->capacity;
    }
    return 0;
    
}

/*
    功能：从已初始化的队列取出最早的字节，写入 *out，更新读下标和数量，写下标不变 
    参数：
        @rb:指向可写结构体
        @out：取出的值的地址
    返回值：
        成功 0；任一指针为 NULL 返回 -1；空返回 -2 

*/
int rb_pop(ring_buf_t *rb, uint8_t *out)
{
    if((rb == NULL)|| (out == NULL))
    {
        return -1;
    }
    if(rb->count == 0)
    {
        return -2;
    }
    if(rb->count > 0)
    {
        *out = rb->data[rb->read_pos];
        rb->count--;
        rb->read_pos = (rb->read_pos + 1) % rb->capacity;
    }
    return 0;
}

