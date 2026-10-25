#include "task21_ring.h"

/*
    功能：初始化接收环形缓冲区
    参数：
        @ring：环形缓冲区地址
        @storage：指向至少 capacity 字节的可写数组
        @capacity：环形缓冲区容量
    返回值：参数错误返回-1，初始化成功返回0
*/
int rx_ring_init(rx_ring_t *ring, uint8_t *storage, size_t capacity)
{
    if((ring == NULL)||(storage == NULL)||(capacity == 0))
    {
        return -1;
    }
    ring->capacity = capacity;
    ring->count = 0U;
    ring->data = storage;
    ring->read_pos = 0U; 
    ring->write_pos = 0U;
    return 0;
}

/*
    功能：向环形缓冲区写入一个数据
    参数：
        @ring：环形缓冲区地址
        @byte：要写入的数据
    返回值：参数错误返回-1，缓冲区满返回-2，初始化成功返回0
*/
int rx_ring_push(rx_ring_t *ring, uint8_t byte)
{
    if((ring == NULL)||(ring->capacity == 0))
    {
        return -1;
    }
    if(ring->count == ring->capacity)
    {
        return -2;
    }
    ring->count++;
    ring->data[ring->write_pos] = byte;
    ring->write_pos = (ring->write_pos + 1) % ring->capacity;
    return 0;
}

/*
    功能：从环形缓冲区读出一个数据
    参数：
        @ring：环形缓冲区地址
        @byte_out：从环形缓冲区读出的数据
    返回值：参数错误返回-1，缓冲区空返回-2，初始化成功返回0
*/
int rx_ring_pop(rx_ring_t *ring, uint8_t *byte_out)
{
    if((ring == NULL)||(byte_out == NULL)||(ring->capacity == 0))
    {
        return -1;
    }
    if(ring->count == 0U)
    {
        return -2;
    }
    ring->count--;
    *byte_out = ring->data[ring->read_pos];
    ring->read_pos = (ring->read_pos + 1) % ring->capacity;
    return 0;
}

