#include "task16.c"
#include <stdio.h>

static int check_empty(const int_list_t *list)
{
    return list->head == NULL && list->tail == NULL && list->size == 0;
}

static int check_values(const int_list_t *list,
                        const int *values,
                        size_t count)
{
    const list_node_t *node = list->head;

    for (size_t i = 0; i < count; i++)
    {
        if (node == NULL || node->value != values[i])
        {
            return 0;
        }
        node = node->next;
    }

    return node == NULL && list->tail != NULL &&
           list->tail->next == NULL && list->size == count;
}

static int check(int condition, const char *name)
{
    if (!condition)
    {
        printf("FAIL: %s\n", name);
        return 0;
    }
    printf("PASS: %s\n", name);
    return 1;
}

int main(void)
{
    int_list_t list;
    size_t index = 999;
    int values[] = {10, 20, 10, 30};
    int ok = 1;

    ok &= check(int_list_init(NULL) == -1, "init NULL");
    ok &= check(int_list_init(&list) == 0 && check_empty(&list),
                "init empty list");
    ok &= check(int_list_find(&list, 10, &index) == -2 && index == 999,
                "find in empty list");
    ok &= check(int_list_push_back(NULL, 1) == -1, "push NULL");

    for (size_t i = 0; i < sizeof(values) / sizeof(values[0]); i++)
    {
        ok &= check(int_list_push_back(&list, values[i]) == 0,
                    "push value");
    }
    ok &= check(check_values(&list, values, 4),
                "list after push_back");

    index = 999;
    ok &= check(int_list_find(&list, 10, &index) == 0 && index == 0,
                "find first duplicate");
    ok &= check(int_list_find(&list, 30, &index) == 0 && index == 3,
                "find tail value");
    ok &= check(int_list_find(&list, 99, &index) == -2 && index == 3,
                "find missing value keeps output");
    ok &= check(int_list_find(&list, 10, NULL) == -1,
                "find NULL output");

    ok &= check(int_list_remove_first(&list, 10) == 0,
                "remove head");
    {
        const int expected[] = {20, 10, 30};
        ok &= check(check_values(&list, expected, 3),
                    "list after removing head");
    }
    ok &= check(int_list_remove_first(&list, 10) == 0,
                "remove middle");
    {
        const int expected[] = {20, 30};
        ok &= check(check_values(&list, expected, 2),
                    "list after removing middle");
    }
    ok &= check(int_list_remove_first(&list, 30) == 0,
                "remove tail");
    {
        const int expected[] = {20};
        ok &= check(check_values(&list, expected, 1),
                    "list after removing tail");
    }
    ok &= check(int_list_remove_first(&list, 20) == 0 && check_empty(&list),
                "remove only node");
    ok &= check(int_list_remove_first(&list, 20) == -2 && check_empty(&list),
                "remove missing value");
    ok &= check(int_list_remove_first(NULL, 20) == -1,
                "remove NULL");

    ok &= check(int_list_clear(&list) == 0 && check_empty(&list),
                "clear empty list");
    ok &= check(int_list_push_back(&list, 42) == 0 &&
                list.head == list.tail && list.head->value == 42,
                "reuse after clear");
    ok &= check(int_list_clear(&list) == 0 && check_empty(&list),
                "clear populated list");
    ok &= check(int_list_clear(NULL) == -1, "clear NULL");

    printf("%s\n", ok ? "ALL TESTS PASSED" : "TESTS FAILED");
    return ok ? 0 : 1;
}
