#include "task17_counter.h"
#include <stdio.h>

/*
    counter_create(10)       -> 成功
    counter_get              -> 读出 10
    counter_add(5)           -> 读出 15
    counter_reset(2)         -> 读出 2
    counter_get(NULL, &out)  -> -1
    counter_get(counter, NULL)-> -1
    counter_add(NULL, 1)     -> -1
    counter_reset(NULL, 1)   -> -1
    counter_destroy(counter) -> 正常释放
    counter_destroy(NULL)    -> 不崩溃
*/
int main(void)
{
    uint32_t val;
    uint32_t out;
    counter_t *counter;
    counter = counter_create(10);
    if (counter == NULL)
    {
        printf("counter_create failed\n");
        return 1;
    }
    if (counter_get(counter, &val) != 0 || val != 10)
    {
        return 1;
    }
    if(counter_add(counter, 5) != 0)
    {
        return 1;
    }
    if (counter_get(counter, &val) != 0 || val != 15)
    {
        return 1;
    }
    if(counter_reset(counter, 2) != 0)
    {
        return 1;
    }
    if (counter_get(counter, &val) != 0 || val != 2)
    {
        return 1;
    }
    if (counter_get(NULL, &out) != -1)
    {
        printf("counter_get NULL test failed\n");
        return 1;
    }
    if(counter_get(counter, NULL) != -1)
    {
        printf("counter_get NULL test failed\n");
        return 1;
    }
    if(counter_add(NULL, 1) != -1)
    {
        printf("counter_add NULL test failed\n");
        return 1;
    }
    if(counter_reset(NULL, 1) != -1)
    {
        printf("counter_reset NULL test failed\n");
        return 1;
    }
    
    counter_destroy(counter);
    counter_destroy(NULL);
    return 0;
}
