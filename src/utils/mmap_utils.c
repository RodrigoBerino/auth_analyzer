#include "mmap_utils.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>

//O mmap() simplifica a leitura e permite particionar o arquivo por ponteiros.


MappedFile map_file(const char *path) {
    /* mmap permite tratar o arquivo como um grande vetor de chars. Isso evita
       ler linha por linha com fgets antes do processamento paralelo. */
    MappedFile mf = { NULL, 0 };

    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        perror("open");
        return mf;
    }

    struct stat st;
    if (fstat(fd, &st) < 0) {
        perror("fstat");
        close(fd);
        return mf;
    }

    mf.size = (size_t)st.st_size;
    mf.data = mmap(NULL,
                    mf.size,
                    PROT_READ,
                    MAP_PRIVATE,
                    fd, 0);
    if (mf.data == MAP_FAILED) {
        perror("mmap");
        mf.data = NULL;
        mf.size = 0;
    }

    // fd pode ser fechado; o mapeamento criado pelo mmap permanece valido
    close(fd);
    return mf;
}


void unmap_file(MappedFile *mf) {
    if (mf->data) {
        munmap(mf->data, mf->size);
        mf->data = NULL;
        mf->size = 0;
    }
}
