#ifndef PEER_STORAGE_H
#define PEER_STORAGE_H

#include <stdint.h>
#include "list.h"
#include "hash_table.h"

#pragma pack(push, 1)

struct t_peer {
    uint32_t id;
    uint8_t peerid[6];
};

struct t_peer_storage {
    uint32_t capacity;
    uint32_t expire;
    uint32_t time_origin;
    uint8_t * cycle_set;
    uint32_t conflict;
    struct t_list * item_list;
    struct t_hash_table * info_table;
};

typedef void (*ps_consumer)(const uint8_t infohash[20], const uint32_t slot, const void * token);

struct t_ps_list_token {
    struct t_peer_storage * ps;
    ps_consumer consumer;
    uint8_t tc;
    const void * token;
};

#pragma pack(pop)

struct t_peer_storage * ps_create(const uint32_t capacity, const uint32_t info_size, const uint32_t expire);

uint32_t ps_put(struct t_peer_storage * ps, uint8_t infohash[20], uint8_t peerid[6]);

void ps_collect(struct t_peer_storage * ps, const ps_consumer consumer, const void * token);

uint32_t ps_size(struct t_peer_storage * ps);

void ps_destroy(struct t_peer_storage * ps);

#endif