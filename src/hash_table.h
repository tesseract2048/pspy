#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <stdint.h>
#include <pthread.h>

#pragma pack(push, 1)

struct t_hash_node {
    uint8_t infohash[20];
};

struct t_hash_table {
    volatile uint8_t * used;
    struct t_hash_node * slot;
    uint32_t size;
};

#pragma pack(pop)

struct t_hash_table * ht_create(const uint32_t size);

uint32_t ht_lookup_or_create(struct t_hash_table * ht, const uint8_t infohash[20]);

uint8_t * ht_get(struct t_hash_table * ht, const uint32_t slot);

void ht_destroy(struct t_hash_table * ht);

#endif