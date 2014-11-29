#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include "list.h"

#define MIN(x,y) ((x < y) ? (x) : (y))

struct t_list_page * list_new_page() {
    struct t_list_page * page = (struct t_list_page *)malloc(sizeof(struct t_list_page));
    bzero(page, sizeof(struct t_list_page));
    return page;
}

struct t_list * list_create() {
    struct t_list * list = (struct t_list *)malloc(sizeof(struct t_list));
    list->size = 0;
    list->last_page = list_new_page();
    return list;
}

void list_destroy(struct t_list * list) {
    struct t_list_page * page = (struct t_list_page *)list->last_page;
    while (page) {
        struct t_list_page * prev = page->prev;
        free(page);
        page = prev;
    }
    free(list);
}

void list_insert(struct t_list * list, const uint32_t id, const uint32_t slot) {
    struct t_list_page * page;
    uint32_t used;
    while (1) {
        page = (struct t_list_page *)list->last_page;
        /* atmoic increase used of current page */
        used = __sync_fetch_and_add(&list->last_page->used, 1);
        /* page is not full, use obtained slot */
        if (used < LIST_PAGE_SIZE) {
            break;
        }
        /* page is full */
        if (used == LIST_PAGE_SIZE) {
            /* has to be the first thread, allocate new page */
            page = list_new_page();
            page->prev = (struct t_list_page *)list->last_page;
            list->last_page = page;
            /* we have no idea whether used has been modified, continue loop */
        }
    }
    struct t_list_element * element = &page->item[used];
    element->id = id;
    element->slot = slot;
    /* atmoc increase size */
    __sync_fetch_and_add(&list->size, 1);
}

void list_foreach(struct t_list * list, const list_consumer consumer, void * token) {
    struct t_list_page * page = (struct t_list_page *)list->last_page;
    while (page) {
        int i;
        int size = MIN(page->used, LIST_PAGE_SIZE);
        for (i = 0; i < size; i ++) {
            (*consumer)(&page->item[i], token);
        }
        page = page->prev;
    }
}
