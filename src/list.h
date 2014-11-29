#ifndef LIST_H
#define LIST_H

#include <stdint.h>

#define LIST_PAGE_SIZE 4096

#pragma pack(push, 1)

struct t_list_element {
    uint32_t id;
    uint32_t slot;
};

struct t_list_page {
    volatile uint32_t used;
    struct t_list_element item[LIST_PAGE_SIZE];
    struct t_list_page * prev;
};

struct t_list {
    uint32_t size;
    volatile struct t_list_page * last_page;
};

#pragma pack(pop)

typedef void (*list_consumer)(struct t_list_element * element, void * token);

struct t_list * list_create();

void list_destroy(struct t_list * list);

void list_insert(struct t_list * list, const uint32_t id, const uint32_t slot);

void list_foreach(struct t_list * list, const list_consumer consumer, void * token);

#endif