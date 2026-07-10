#include "hashtable.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


static unsigned int djb2(const char *key) {
    /* Hash simples para transformar uma chave textual, como um IP, em indice
       de bucket. */
    unsigned int hash = 5381;
    int c;
    while ((c = (unsigned char)*key++))
        hash = ((hash << 5) + hash) + (unsigned int)c;
    return hash;
}

HashTable *ht_create(int n_buckets) {
    HashTable *ht = malloc(sizeof(HashTable));
    if (!ht) return NULL;
    ht->n_buckets = n_buckets;
    ht->buckets   = calloc((size_t)n_buckets, sizeof(Entry *));
    if (!ht->buckets) { free(ht); return NULL; }
    return ht;
}


void ht_increment(HashTable *ht, const char *key) {
    int idx = (int)(djb2(key) % (unsigned int)ht->n_buckets);

    /* Se a chave ja existe no bucket, incrementa a contagem. */
    for (Entry *e = ht->buckets[idx]; e; e = e->next) {
        if (strcmp(e->key, key) == 0) {
            e->count++;
            return;
        }
    }

    /* Se a chave ainda nao existe, cria uma nova entrada na lista do bucket. */
    Entry *e = malloc(sizeof(Entry));
    if (!e) return;
    e->key   = strdup(key);
    e->count = 1;
    e->next  = ht->buckets[idx];
    ht->buckets[idx] = e;
}



void ht_merge(HashTable *dst, HashTable *src) {
    /* Reduce do projeto: percorre cada entrada local e soma sua contagem na
       tabela global. Esta funcao nao libera src; o chamador faz isso. */
    for (int i = 0; i < src->n_buckets; i++) {
        for (Entry *e = src->buckets[i]; e; e = e->next) {
            int idx = (int)(djb2(e->key) % (unsigned int)dst->n_buckets);

            for (Entry *d = dst->buckets[idx]; d; d = d->next) {
                if (strcmp(d->key, e->key) == 0) {
                    d->count += e->count;
                    goto next_entry;
                }
            }
            {
                Entry *n = malloc(sizeof(Entry));
                if (!n) continue;
                n->key   = strdup(e->key);
                n->count = e->count;
                n->next  = dst->buckets[idx];
                dst->buckets[idx] = n;
            }
            next_entry:;
        }
    }
}

static int cmp_entries(const void *a, const void *b) {
    Entry *ea = *(Entry **)a;
    Entry *eb = *(Entry **)b;
    if (eb->count > ea->count) return 1;
    if (eb->count < ea->count) return -1;
    return strcmp(ea->key, eb->key);
}

Entry **ht_entries_sorted(HashTable *ht, int *out_total) {
    /* Converte as listas encadeadas dos buckets em um vetor ordenavel. Isso e
       usado apenas na saida, para imprimir o Top N. */
    int total = 0;
    for (int i = 0; i < ht->n_buckets; i++)
        for (Entry *e = ht->buckets[i]; e; e = e->next)
            total++;

    if (out_total) *out_total = total;
    if (total == 0) return NULL;

    Entry **arr = malloc((size_t)total * sizeof(Entry *));
    if (!arr) return NULL;

    int j = 0;
    for (int i = 0; i < ht->n_buckets; i++)
        for (Entry *e = ht->buckets[i]; e; e = e->next)
            arr[j++] = e;

    qsort(arr, (size_t)total, sizeof(Entry *), cmp_entries);
    return arr;
}

void ht_destroy(HashTable *ht) {
    if (!ht) return;
    for (int i = 0; i < ht->n_buckets; i++) {
        Entry *e = ht->buckets[i];
        while (e) {
            Entry *next = e->next;
            free(e->key);
            free(e);
            e = next;
        }
    }
    free(ht->buckets);
    free(ht);
}
