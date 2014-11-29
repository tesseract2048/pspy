#include <stdlib.h>
#include <strings.h>
#include <stdbool.h>
#include <zlib.h>
#include <stdio.h>
#include "hash_table.h"

#define BYTE_POS(slot) ((slot) / 8)
#define BIT_POS(slot) ((slot) % 8)

struct t_hash_table * ht_create(const uint32_t size) {
    struct t_hash_table * ht = (struct t_hash_table *)malloc(sizeof(struct t_hash_table));
    bzero(ht, sizeof(struct t_hash_table));
    ht->used = (uint8_t *)malloc(size / 8 + 1);
    ht->slot = (struct t_hash_node *)malloc(sizeof(struct t_hash_node) * size);
    ht->size = size;
    return ht;
}

bool ht_acquire_slot(struct t_hash_table * ht, const uint8_t infohash[20], const uint32_t slot) {
    /* may miss to lookup due to multithread issue, which would not affect functionality */
    if (memcmp(ht->slot[slot].infohash, infohash, 20) == 0) {
        return true;
    }
    while (1) {
        uint8_t oldval = ht->used[BYTE_POS(slot)];
        uint8_t newval = oldval | (1 << BIT_POS(slot));
        if (oldval & (1 << BIT_POS(slot))) {
            /* failed to acquire, already occupied */
            return false;
        }
        /* try set used bit to 1 with CAS */
        if (__sync_bool_compare_and_swap(&ht->used[BYTE_POS(slot)], oldval, newval)) {
            memcpy(&ht->slot[slot].infohash, infohash, 20);
            return true;
        }
    }
}

uint32_t ht_lookup_or_create(struct t_hash_table * ht, const uint8_t infohash[20]) {
    uint32_t slot = crc32(0, (const unsigned char *)infohash, 20) % ht->size;
    while (!ht_acquire_slot(ht, infohash, slot)) {
        slot = (slot + 37) % ht->size;
    }
    memcpy(&ht->slot[slot].infohash, infohash, 20);
    return slot;
}

uint8_t * ht_get(struct t_hash_table * ht, const uint32_t slot) {
    return (uint8_t *)&ht->slot[slot].infohash;
}

void ht_destroy(struct t_hash_table * ht) {
    free((void*)ht->used);
    free(ht->slot);
    free(ht);
}
