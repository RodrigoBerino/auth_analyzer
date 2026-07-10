#include <stdio.h>
#include <stdlib.h>

#include "auth_analyzer.h"

int main(int argc, char *argv[]) {
    /* Entrada do programa: valida os argumentos e entrega o trabalho ao
       modulo que orquestra o MapReduce. */
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <arquivo.log> <n_threads>\n", argv[0]);
        return 1;
    }

    int n_threads = atoi(argv[2]);
    if (n_threads < 1) {
        fprintf(stderr, "Erro: <n_threads> deve ser maior que zero.\n");
        return 1;
    }

    return analyze_auth_log(argv[1], n_threads);
}
