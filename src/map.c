#include "map.h"

#include "utils/log_parser.h"
/*Map: cada thread conta em uma hash table local
Responsabilidade de map.c
percorre apenas o bloco da
thread;
encontra o fim de cada linha;
verifica se a linha contém
Failed;
extrai o IP;
incrementa a tabela local.*/



void *map_worker(void *arg) {
    /* Funcao executada por uma thread. Ela percorre apenas o bloco recebido e
       grava os resultados na HashTable local daquela thread. */
    MapArgs *a   = (MapArgs *)arg;
    char    *pos = a->block.start;
    char    *end = a->block.end;
    char     ip[256];

    while (pos < end) {
        char *eol = pos;
        while (eol < end && *eol != '\n')
            eol++;

        size_t line_len = (size_t)(eol - pos);

        if (line_len > 0 && log_line_is_failed_auth(pos, line_len)) {
            if (log_line_extract_ip(pos, line_len, ip, sizeof(ip)) == 0) {
                /* Resultado do MAP: "IP -> quantidade de falhas" local. */
                ht_increment(a->ht, ip);
            }
        }

        pos = eol + 1;
    }
    return NULL;
}
