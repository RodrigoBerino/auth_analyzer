#ifndef MAP_H
#define MAP_H

#include "split.h"
#include "hashtable.h"

// cada thread recebe seu proprio bloco + hash table local (sem mutex)
typedef struct {
    Block      block;
    HashTable *ht;
} MapArgs;

void *map_worker(void *arg);

#endif /* MAP_H */
