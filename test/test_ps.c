#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include "peer_storage.h"

uint32_t size = 0;

void consumer(const uint8_t infohash[20], const uint32_t slot, const void * token) {
    size ++;
}

int main(int argc, char ** argv) {
    double write[] = {0.01, 0.05, 0.1, 0.2, 0.3, 0.5, 1.0, 2.0, 4.0, 5.0};
    int capacity = 100000000;
    int i;
    for (i = 0; i < 9; i ++) {
        struct t_peer_storage * ps = ps_create(capacity, 10000000, 4);
        uint32_t ts = time(NULL);
        int tot = capacity * write[i];
        int ih = tot / 100;
        int i, j, k;
        srand(time(NULL));
        for (i = 0; i < ih; i ++) {
            uint8_t infohash[20];
            for (j = 0; j < 20; j ++) {
                infohash[j] = 'a' + rand() % 26;
            }
            for (j = 0; j < 100; j ++) {
                uint8_t peerid[6];
                for (k = 0; k < 6; k ++) {
                    peerid[k] = rand() % 256;
                }
                ps_put(ps, infohash, peerid);
            }
        }
        printf("%d\t%d\n", ps_size(ps), ps->conflict);
        // printf("put done: %d\n", time(NULL) - ts);
        // printf("put: %d\n", ps_size(ps));
        // printf("conflict: %d\n", ps->conflict);
        size = 0;
        ps_collect(ps, consumer, NULL);
        if (ps_size(ps) != size) {
            printf("size mismatch: %d / %d\n", ps_size(ps), size);
        }
        ts = time(NULL);
        // printf("collect done: %d\n", time(NULL) - ts);
        ps_destroy(ps);
    }
}
