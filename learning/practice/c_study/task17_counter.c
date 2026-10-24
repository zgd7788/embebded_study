#include "task17_counter.h"
#include <stdlib.h>

struct counter
{
    uint32_t cnt;
};
/*
    功能：动态创建一个计数器，并设置初始值。
    参数：
        @initial：计数器的初始值。
    返回值：
        成功返回计数器对象指针；
        内存分配失败返回 NULL。
*/
counter_t *counter_create(uint32_t initial)
{
    counter_t *counter = malloc(sizeof(*counter));
    if(counter == NULL)
    {
        return NULL;
    }
    counter->cnt = initial;
    return counter; 
}

/*
    功能：销毁计数器并释放动态内存。
    参数：
        @counter：由 counter_create 返回的计数器指针；
                  允许为 NULL。
    返回值：
        无。
*/
void counter_destroy(counter_t *counter)
{
    free(counter);
}

/*
    功能：将指定增量加到计数器当前值上。
    参数：
        @counter：有效的计数器指针；
        @delta：要增加的数值。
    返回值：
        成功返回 0；
        counter == NULL 返回 -1。
*/
int counter_add(counter_t *counter, uint32_t delta)
{
    if(counter == NULL)
    {
        return -1;
    }
    counter->cnt += delta;
    return 0;
}
/*
    功能：读取计数器当前值。
    参数：
        @counter：有效的计数器指针；
        @value_out：用于接收当前值的输出地址。
    返回值：
        成功返回 0；
        任一指针为 NULL 返回 -1。
*/
int counter_get(const counter_t *counter, uint32_t *value_out)
{
    if((counter == NULL)||(value_out == NULL))
    {
        return -1;
    }
    *value_out = counter->cnt;
    return 0;
}

/*
    功能：将计数器当前值重置为指定值。
    参数：
        @counter：有效的计数器指针；
        @value：新的计数值。
    返回值：
        成功返回 0；
        counter == NULL 返回 -1。
*/
int counter_reset(counter_t *counter, uint32_t value)
{
    if(counter == NULL)
    {
        return -1;
    }
    counter->cnt = value;
    return 0;
}