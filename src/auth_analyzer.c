#include "auth_analyzer.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "hashtable.h"
#include "map.h"
#include "split.h"
#include "utils/mmap_utils.h"
#include "utils/report.h"
#include "utils/timer_utils.h"

#define HASH_BUCKETS 8192

/* Fase MAP:
   - cada thread recebe um bloco independente do arquivo;
   - cada thread escreve em sua propria HashTable;
   - por isso nao precisamos de mutex no MVP. */
static int run_map_phase(Block *blocks, int n_threads, MapArgs *args, pthread_t *threads) {
    int created = 0;

    for (int i = 0; i < n_threads; i++) {
        args[i].block = blocks[i];
        args[i].ht = ht_create(HASH_BUCKETS);
        if (!args[i].ht) {
            fprintf(stderr, "Erro: nao foi possivel criar hash table local.\n");
            break;
        }

        if (pthread_create(&threads[i], NULL, map_worker, &args[i]) != 0) {
            fprintf(stderr, "Erro: nao foi possivel criar thread %d.\n", i);
            ht_destroy(args[i].ht);
            args[i].ht = NULL;
            break;
        }

        created++;
    }

    for (int i = 0; i < created; i++) {
        pthread_join(threads[i], NULL);
    }

    return created == n_threads ? 0 : -1;
}

/* Fase REDUCE/MERGE:
   junta as tabelas locais em uma tabela global.
   A primeira tabela vira o acumulador para evitar uma copia inicial. */
static HashTable *merge_results(MapArgs *args, int n_threads) {
    HashTable *global = args[0].ht;
    args[0].ht = NULL;

    for (int i = 1; i < n_threads; i++) {
        ht_merge(global, args[i].ht);
        ht_destroy(args[i].ht);
        args[i].ht = NULL;
    }

    return global;
}

static void destroy_local_tables(MapArgs *args, int n_threads) {
    if (!args) return;

    for (int i = 0; i < n_threads; i++) {
        ht_destroy(args[i].ht);
        args[i].ht = NULL;
    }
}

int analyze_auth_log(const char *path, int n_threads) {
    AnalysisStats stats = {0};
    HashTable *global = NULL;
    Block *blocks = NULL;
    MapArgs *args = NULL;
    pthread_t *threads = NULL;
    int status = 1;

    /* 1) Leitura eficiente: mmap coloca o arquivo na memoria virtual. */
    double t0 = timer_now();
    MappedFile mf = map_file(path);
    if (!mf.data) return 1;

    /* 2) SPLIT: cria blocos alinhados por linha para cada thread. */
    blocks = split_file(mf.data, mf.size, n_threads);
    if (!blocks) {
        fprintf(stderr, "Erro: split falhou.\n");
        goto cleanup;
    }
    double t1 = timer_now();

    threads = malloc((size_t)n_threads * sizeof(pthread_t));
    args = calloc((size_t)n_threads, sizeof(MapArgs));
    if (!threads || !args) {
        fprintf(stderr, "Erro: memoria insuficiente.\n");
        goto cleanup;
    }

    /* 3) MAP: as threads contam IPs com falha em tabelas locais. */
    if (run_map_phase(blocks, n_threads, args, threads) != 0) {
        goto cleanup;
    }
    double t2 = timer_now();

    /* 4) REDUCE: combina as contagens locais em uma tabela global. */
    global = merge_results(args, n_threads);
    double t3 = timer_now();

    stats.path = path;
    stats.threads = n_threads;
    stats.lines = count_lines(mf.data, mf.size);
    stats.split_seconds = t1 - t0;
    stats.map_seconds = t2 - t1;
    stats.merge_seconds = t3 - t2;
    stats.total_seconds = t3 - t0;

    print_analysis_report(global, &stats);
    status = 0;

cleanup:
    /* Caminho unico de limpeza: funciona tanto em sucesso quanto em erro. */
    ht_destroy(global);
    destroy_local_tables(args, n_threads);
    free(args);
    free(threads);
    free(blocks);
    unmap_file(&mf);
    return status;
}
