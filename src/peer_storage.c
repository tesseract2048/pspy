#include <stdlib.h>
#include <strings.h>
#include <time.h>
#include <zlib.h>
#include <math.h>
#include "list.h"
#include "peer_storage.h"

#define SLOT_BITS 4
#define SLOT_PER_BYTE (8 / SLOT_BITS)
#define BYTE_POS(slot) ((slot) / SLOT_PER_BYTE)
#define BIT_POS(slot) (((slot) % SLOT_PER_BYTE) * SLOT_BITS)

struct t_peer_storage * ps_create(const uint32_t capacity, const uint32_t info_size, const uint32_t expire) {
    struct t_peer_storage * ps = (struct t_peer_storage *)malloc(sizeof(struct t_peer_storage));
    bzero(ps, sizeof(struct t_peer_storage));
    ps->capacity = capacity;
    ps->expire = expire;
    ps->time_origin = (uint32_t)time(NULL);
    ps->cycle_set = (uint8_t *)malloc(capacity / SLOT_PER_BYTE);
    bzero(ps->cycle_set, capacity / SLOT_PER_BYTE);
    ps->item_list = list_create();
    ps->info_table = ht_create(info_size);
    ps->conflict = 0;
    return ps;
}

uint8_t ps_time_cycle(const struct t_peer_storage * ps) {
    return ((uint32_t)time(NULL) - ps->time_origin) / 3600 % 254 + 1;
}

uint32_t ps_slot(const struct t_peer_storage * ps, uint32_t id, uint8_t peerid[6]) {
    struct t_peer peer;
    peer.id = id;
    memcpy(peer.peerid, peerid, 6);
    return crc32(0, (const unsigned char *)&peer, sizeof(struct t_peer)) % ps->capacity;
}

uint8_t ps_slot_acquire(struct t_peer_storage * ps, uint32_t slot) {
    uint8_t mask = 0x0F << BIT_POS(slot);
    uint8_t val = __sync_fetch_and_or((uint8_t *)&ps->cycle_set[BYTE_POS(slot)], mask) & mask;
    val >>= BIT_POS(slot);
    return val;
}

uint8_t ps_slot_read(struct t_peer_storage * ps, uint32_t slot) {
    uint8_t mask = 0x0F << BIT_POS(slot);
    uint8_t val = ps->cycle_set[BYTE_POS(slot)] & mask;
    val >>= BIT_POS(slot);
    return val;
}

void ps_slot_update(struct t_peer_storage * ps, uint32_t slot, uint8_t val) {
    uint8_t mask = 0xFF - (0x0F << BIT_POS(slot));
    mask |= (val << BIT_POS(slot));
    __sync_fetch_and_and((uint8_t *)&ps->cycle_set[BYTE_POS(slot)], mask);
}

uint32_t ps_put(struct t_peer_storage * ps, uint8_t infohash[20], uint8_t peerid[6]) {
    uint32_t id = ht_lookup_or_create(ps->info_table, infohash);
    uint32_t slot = ps_slot(ps, id, peerid);
    uint8_t tc = ps_time_cycle(ps);
    /* occupy and fetch the first byte, prevent duplicate entry */
    uint8_t val = ps_slot_acquire(ps, slot);
    if (val == 0) {
        /* insert confirmed new item to list */
        list_insert(ps->item_list, id, slot);
    } else {
        /* count confliction, for debugging only */
        ps->conflict ++;
    }
    /* update cycle slot */
    ps_slot_update(ps, slot, tc);
    return slot;
}

void ps_accept(struct t_list_element * element, void * list_token) {
    struct t_ps_list_token * t = (struct t_ps_list_token *)list_token;
    uint32_t slot = element->slot;
    if (abs(ps_slot_read(t->ps, slot) - t->tc) > t->ps->expire) {
        ps_slot_update(t->ps, slot, 0);
        return;
    }
    (*t->consumer)((uint8_t *)ht_get(t->ps->info_table, element->id), element->slot, t->token);
    /* migrate item to new list */
    list_insert(t->ps->item_list, element->id, element->slot);
}

void ps_collect(struct t_peer_storage * ps, const ps_consumer consumer, const void * token) {
    struct t_ps_list_token list_token = {ps, consumer, ps_time_cycle(ps), token};
    struct t_list * list = ps->item_list;
    /* set current list to new one, for writing new items */
    ps->item_list = list_create();
    /* consume and migrate items */
    list_foreach(list, ps_accept, (void *)&list_token);
    /* destroy old list */
    list_destroy(list);
}

uint32_t ps_size(struct t_peer_storage * ps) {
    return ps->item_list->size;
}

void ps_destroy(struct t_peer_storage * ps) {
    list_destroy(ps->item_list);
    ht_destroy(ps->info_table);
    free(ps->cycle_set);
    free(ps);
}