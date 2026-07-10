#ifndef MMAP_UTILS_H
#define MMAP_UTILS_H
#include <stddef.h>

typedef struct {
    char  *data;
    size_t size;
} MappedFile;

MappedFile map_file(const char *path);

void unmap_file(MappedFile *mf);

#endif
