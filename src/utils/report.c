#include "report.h"

#include <stdio.h>
#include <stdlib.h>

#define TOP_IP_LIMIT 10

static double throughput(const AnalysisStats *stats) {
    if (stats->total_seconds <= 0.0) return 0.0;
    return (double)stats->lines / stats->total_seconds;
}

static void print_top_ips(HashTable *ips) {
    int total_ips = 0;
    /* A HashTable e boa para contar, mas nao para imprimir ordenado. Por isso
       pedimos um vetor ja ordenado por quantidade de falhas. */
    Entry **entries = ht_entries_sorted(ips, &total_ips);

    printf("Top %d IPs com mais falhas\n", TOP_IP_LIMIT);
    printf("%-4s %-18s %10s\n", "#", "IP", "Falhas");
    printf("%-4s %-18s %10s\n", "---", "------------------", "----------");

    if (!entries || total_ips == 0) {
        printf("     Nenhuma falha encontrada.\n");
        free(entries);
        return;
    }

    int limit = total_ips < TOP_IP_LIMIT ? total_ips : TOP_IP_LIMIT;
    for (int i = 0; i < limit; i++) {
        printf("%-4d %-18s %10d\n", i + 1, entries[i]->key, entries[i]->count);
    }

    free(entries);
}

static void print_performance(const AnalysisStats *stats) {
    printf("\nDesempenho\n");
    printf("Arquivo    : %s\n", stats->path);
    printf("Threads    : %d\n", stats->threads);
    printf("Linhas     : %zu\n", stats->lines);
    printf("Split      : %.4fs\n", stats->split_seconds);
    printf("Map        : %.4fs\n", stats->map_seconds);
    printf("Merge      : %.4fs\n", stats->merge_seconds);
    printf("Total      : %.4fs\n", stats->total_seconds);
    printf("Throughput : %.0f linhas/s\n", throughput(stats));
}

void print_analysis_report(HashTable *ips, const AnalysisStats *stats) {
    /* Mantem a formatacao da saida isolada do algoritmo MapReduce. */
    printf("\n=== Relatorio de autenticacao ===\n\n");
    print_top_ips(ips);
    print_performance(stats);
}
