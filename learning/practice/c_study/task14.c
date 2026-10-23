#include <stdint.h>

typedef enum 
{
    SM_OFF, 
    SM_ON, 
    SM_FAULT
} sm_state_t;

typedef enum 
{
    SM_EVENT_TURN_ON,
    SM_EVENT_TURN_OFF,
    SM_EVENT_ERROR,
    SM_EVENT_RESET
} sm_event_t;

typedef void (*sm_state_change_cb_t)(sm_state_t old_state, sm_state_t new_state);

typedef struct
{
    sm_state_t state;
    sm_state_change_cb_t on_change;
} sm_t;

void my_callback(sm_state_t old_state, sm_state_t new_state)
{

}

/*
    功能：状态初始化
    参数：
        @sm：
        @initial_state：
        @callback：
    返回值：
        成功返回0，否则返回-1
*/
int sm_init(sm_t *sm, sm_state_t initial_state, sm_state_change_cb_t callback)
{
    if((sm == NULL)||((initial_state != SM_OFF)&&(initial_state != SM_ON)&&(initial_state != SM_FAULT)))
    {
        return -1;
    }
    sm->state = initial_state;
    sm->on_change = callback;
    return 0;
}

/*
    功能：
    参数：
    返回值：
        成功返回0，否则返回-1
*/
int sm_handle_event(sm_t *sm, sm_event_t event)
{
    if((sm == NULL)||((event != SM_EVENT_TURN_ON)&&(event != SM_EVENT_TURN_OFF)&&
        (event != SM_EVENT_ERROR)&&(event != SM_EVENT_RESET)))
    {
        return -1;
    }
    if((sm->state != SM_FAULT)&&(sm->state != SM_OFF)&&(sm->state != SM_ON))
    {
        return -1;
    }
    sm_state_t last_state;
    if(sm->state != SM_FAULT)
    {
        switch(event)
        {
            case SM_EVENT_TURN_ON:
                if(SM_ON != sm->state)
                {
                    last_state = sm->state;
                    sm->state = SM_ON;
                    if(sm->on_change != NULL)
                    {
                        sm->on_change(last_state, SM_ON);
                    }
                }
            break;
            case SM_EVENT_TURN_OFF:
                if(SM_OFF != sm->state)
                {
                    last_state = sm->state;
                    sm->state = SM_OFF;
                    if(sm->on_change != NULL)
                    {
                        sm->on_change(last_state, SM_OFF);
                    }
                }
            break;
            case SM_EVENT_ERROR:
                if(SM_FAULT != sm->state)
                {
                    last_state = sm->state;
                    sm->state = SM_FAULT;
                    if(sm->on_change != NULL)
                    {
                        sm->on_change(last_state, SM_FAULT);
                    }
                }
            break;        
            case SM_EVENT_RESET:
                if(SM_OFF != sm->state)
                {
                    last_state = sm->state;
                    sm->state = SM_OFF;
                    if(sm->on_change != NULL)
                    {
                        sm->on_change(last_state, SM_OFF);
                    }
                }
            break;
            default:
            break;
        }
    }
    else
    {
        if(event == SM_EVENT_RESET)
        {
            last_state = sm->state;
            sm->state = SM_OFF;
            if(sm->on_change != NULL)
            {
                sm->on_change(last_state, SM_OFF);
            }
        }
    }
    return 0;
}
