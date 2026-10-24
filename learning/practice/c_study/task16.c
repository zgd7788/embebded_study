#include <stdlib.h>

typedef struct list_node {
    int value;
    struct list_node *next;
} list_node_t;

typedef struct {
    list_node_t *head;
    list_node_t *tail;
    size_t size;
} int_list_t;

/*
    功能：初始化为空表。
    参数：
    返回值：成功返回0，失败返回-1。
*/
int int_list_init(int_list_t *list)
{
    if(list == NULL)
    {
        return -1;
    }
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
    return 0;
}
/*
    功能：尾部追加节点。
    参数：
    返回值：成功返回0，空指针返回-1，分配失败返回-2
*/
int int_list_push_back(int_list_t *list, int value)
{
    if(list == NULL)
    {
        return -1;
    }
    list_node_t *_next = malloc(sizeof(*_next));
    if(_next == NULL)
    {
        return -2;
    }
    if((list->head == NULL)&&(list->tail==NULL))
    {
        list->head = _next;
        list->tail = _next;
        _next->next = NULL;
    }
    else
    {
        list->tail->next = _next;
        _next->next = NULL;
        list->tail = _next;
    }
    _next->value = value;
    list->size++;
    return 0;
}
/*
    功能：查找第一个匹配值。
    参数：
    返回值：找到返回 0 并写入从 0 开始的下标；指针为空返回 -1；未找到返回 -2。
*/
int int_list_find(const int_list_t *list, int value, size_t *index_out)
{
    if((list == NULL)||(index_out == NULL))
    {
        return -1;
    }
    const list_node_t *_node;
    _node = list->head;
    size_t i=0;
    if(_node == NULL)
    {
        return -2;
    }
    while(_node->value != value)
    {
        if(_node->next != NULL)
        {
            _node = _node->next;
            i++;
        }
        else
        {
            return -2;
        }
    }
    *index_out = i;
    return 0;
}
/*
    功能：删除并释放第一个匹配节点。
    参数：
    返回值：空指针返回 -1；未找到返回 -2 且链表不变；成功返回 0，并正确处理删头、删尾和删唯一节点。
*/
int int_list_remove_first(int_list_t *list, int value)
{
    if(list == NULL)
    {
        return -1;
    }
    list_node_t *_node;
    list_node_t *_last_node = NULL;
    _node = list->head;
    
    if(_node == NULL)
    {
        return -2;
    }
    while(_node->value != value)
    {
        if(_node->next != NULL)
        {
            _last_node = _node;
            _node = _node->next;
        }
        else
        {
            return -2;
        }
    }
    if(_node == list->head)
    {
        /* 头节点value = value*/
        list->head = _node->next;
        if(list->size == 1)
        {
            list->tail = NULL;
        }
    }
    else if(_node == list->tail)
    {
        _last_node->next = NULL;
        list->tail = _last_node;
    }
    else
    {
        _last_node->next = _node->next;
    }
    free(_node);
    list->size--;
    return 0;
}
/*
    功能：释放全部节点并重置为空表。
    参数：
    返回值：空指针返回 -1；成功返回 0。清空后仍可再次插入或清空。
*/
int int_list_clear(int_list_t *list)
{
    if(list == NULL)
    {
        return -1;
    }
    list_node_t *current_node;
    list_node_t *_next_node = NULL;
    current_node = list->head;
    
    if(current_node == NULL)
    {
        return 0;
    }
    while(current_node != NULL)
    {
        _next_node = current_node->next;
        free(current_node);
        current_node = _next_node;
    }
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
    return 0;
}