#ifndef HASHTABLE_H
#define HASHTABLE_H

typedef struct Entry {
    char         *key;
    int           count;
    struct Entry *next;
} Entry;

typedef struct {
    Entry **buckets;
    int     n_buckets;
} HashTable;

HashTable *ht_create(int n_buckets);
void       ht_increment(HashTable *ht, const char *key);
void       ht_merge(HashTable *dst, HashTable *src);
Entry    **ht_entries_sorted(HashTable *ht, int *out_total);
void       ht_destroy(HashTable *ht);

#endif /* HASHTABLE_H */
