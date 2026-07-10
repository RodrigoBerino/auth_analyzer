#ifndef REPORT_H
#define REPORT_H

#include <stddef.h>

#include "hashtable.h"

typedef struct {
    const char *path;
    int threads;
    size_t lines;
    double split_seconds;
    double map_seconds;
    double merge_seconds;
    double total_seconds;
} AnalysisStats;

void print_analysis_report(HashTable *ips, const AnalysisStats *stats);

#endif /* REPORT_H */
