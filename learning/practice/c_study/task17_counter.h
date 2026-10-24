#ifndef TASK17_COUNTER_H_INCLUDED
#define TASK17_COUNTER_H_INCLUDED

#include <stdint.h>

typedef struct counter counter_t;
counter_t *counter_create(uint32_t initial);
void counter_destroy(counter_t *counter);
int counter_add(counter_t *counter, uint32_t delta);
int counter_get(const counter_t *counter, uint32_t *value_out);
int counter_reset(counter_t *counter, uint32_t value);


#endif