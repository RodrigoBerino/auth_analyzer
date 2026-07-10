# Arquitetura do Auth Analyzer

Este projeto usa uma ideia de MapReduce para contar IPs com falhas de login em
um arquivo `auth.log`.

Resumo curto:

```text
arquivo.log
  -> mmap
  -> split em blocos
  -> map em N threads
  -> reduce/merge das contagens
  -> relatorio final
```

## O que e MapReduce aqui

No projeto, MapReduce aparece assim:

1. **Split**
   O arquivo grande e dividido em blocos. Cada bloco fica com uma thread.

2. **Map**
   Cada thread le seu bloco, encontra linhas com `Failed`, extrai o IP e conta
   as falhas em uma hash table local.

3. **Reduce/Merge**
   Depois que todas as threads terminam, o programa junta as hash tables locais
   em uma hash table global.

4. **Output**
   A hash table global e ordenada para imprimir o Top IPs com mais falhas.

## Estrutura de pastas

```text
src/
├── main.c
├── auth_analyzer.c
├── auth_analyzer.h
├── split.c
├── split.h
├── map.c
├── map.h
├── hashtable.c
├── hashtable.h
└── utils/
    ├── mmap_utils.c
    ├── mmap_utils.h
    ├── log_parser.c
    ├── log_parser.h
    ├── report.c
    ├── report.h
    ├── timer_utils.c
    └── timer_utils.h
```

## Arquivos principais

### `src/main.c`

Responsabilidade:

- recebe os argumentos do terminal;
- valida se o usuario passou `<arquivo.log>` e `<n_threads>`;
- chama `analyze_auth_log()`.

Funcao principal:

- `main(int argc, char *argv[])`

Este arquivo nao implementa MapReduce. Ele so e a porta de entrada do programa.

### `src/auth_analyzer.c`

Responsabilidade:

- orquestra o fluxo completo;
- chama mmap;
- chama split;
- cria threads;
- chama map;
- chama reduce/merge;
- chama o relatorio.

Funcoes principais:

- `analyze_auth_log(const char *path, int n_threads)`
  Executa o programa inteiro.

- `run_map_phase(...)`
  Cria as threads. Cada thread executa `map_worker()`.

- `merge_results(...)`
  Junta as hash tables locais em uma hash table global.

- `destroy_local_tables(...)`
  Libera tabelas locais caso ocorra erro.

Este e o melhor arquivo para explicar o fluxo geral do projeto.

### `src/split.c`

Responsabilidade:

- dividir o arquivo em blocos;
- garantir que nenhuma linha de log fique cortada entre duas threads.

Funcao principal:

- `split_file(char *data, size_t size, int n_blocks)`

Entrada:

- ponteiro para o arquivo em memoria;
- tamanho do arquivo;
- numero de blocos, normalmente igual ao numero de threads.

Saida:

- vetor de `Block`.

Cada `Block` tem:

- `start`: inicio do bloco;
- `end`: fim do bloco;
- `block_id`: identificador do bloco.

### `src/map.c`

Responsabilidade:

- processar um bloco de linhas;
- identificar falhas de autenticacao;
- extrair IPs;
- incrementar a hash table local.

Funcao principal:

- `map_worker(void *arg)`

Ela recebe um `MapArgs`, que contem:

- `block`: bloco que a thread deve processar;
- `ht`: hash table local da thread.

Importante:

- cada thread tem sua propria hash table;
- por isso a fase map nao precisa de mutex;
- o merge so acontece depois que todas as threads terminam.

### `src/hashtable.c`

Responsabilidade:

- guardar contagens no formato `IP -> quantidade`;
- incrementar contagens durante o map;
- juntar tabelas durante o reduce;
- devolver entradas ordenadas para o relatorio.

Funcoes principais:

- `ht_create(int n_buckets)`
  Cria uma hash table.

- `ht_increment(HashTable *ht, const char *key)`
  Incrementa a contagem de um IP.

- `ht_merge(HashTable *dst, HashTable *src)`
  Soma os dados de `src` dentro de `dst`. Esta e a parte de reduce.

- `ht_entries_sorted(HashTable *ht, int *out_total)`
  Cria um vetor ordenado para imprimir o Top IPs.

- `ht_destroy(HashTable *ht)`
  Libera memoria da tabela.

## Arquivos utils

Os arquivos em `src/utils/` nao sao o centro do MapReduce. Eles existem para
deixar o codigo principal menor e mais facil de explicar.

### `src/utils/mmap_utils.c`

Responsabilidade:

- abrir o arquivo com `mmap`;
- liberar o mapeamento depois.

Funcoes:

- `map_file(const char *path)`
- `unmap_file(MappedFile *mf)`

Uso no fluxo:

```text
auth_analyzer.c -> map_file()
auth_analyzer.c -> unmap_file()
```

### `src/utils/log_parser.c`

Responsabilidade:

- analisar uma linha de log;
- dizer se a linha representa falha;
- extrair o IP da linha.

Funcoes:

- `log_line_is_failed_auth(const char *line, size_t len)`
- `log_line_extract_ip(const char *line, size_t len, char *ip, size_t ip_size)`

Uso no fluxo:

```text
map.c -> log_line_is_failed_auth()
map.c -> log_line_extract_ip()
```

### `src/utils/report.c`

Responsabilidade:

- formatar a saida no terminal;
- imprimir Top IPs;
- imprimir metricas de desempenho.

Funcao principal:

- `print_analysis_report(HashTable *ips, const AnalysisStats *stats)`

Uso no fluxo:

```text
auth_analyzer.c -> print_analysis_report()
report.c -> ht_entries_sorted()
```

### `src/utils/timer_utils.c`

Responsabilidade:

- medir tempo;
- contar linhas do arquivo.

Funcoes:

- `timer_now()`
- `count_lines(const char *data, size_t size)`

Uso no fluxo:

```text
auth_analyzer.c -> timer_now()
auth_analyzer.c -> count_lines()
```

## Quem chama quem

Fluxo completo:

```text
main.c
  -> analyze_auth_log()                         src/auth_analyzer.c
      -> timer_now()                            src/utils/timer_utils.c
      -> map_file()                             src/utils/mmap_utils.c
      -> split_file()                           src/split.c
      -> run_map_phase()                        src/auth_analyzer.c
          -> pthread_create()
              -> map_worker()                   src/map.c
                  -> log_line_is_failed_auth()  src/utils/log_parser.c
                  -> log_line_extract_ip()      src/utils/log_parser.c
                  -> ht_increment()             src/hashtable.c
          -> pthread_join()
      -> merge_results()                        src/auth_analyzer.c
          -> ht_merge()                         src/hashtable.c
      -> count_lines()                          src/utils/timer_utils.c
      -> print_analysis_report()                src/utils/report.c
          -> ht_entries_sorted()                src/hashtable.c
      -> ht_destroy()                           src/hashtable.c
      -> unmap_file()                           src/utils/mmap_utils.c
```

## Como explicar em apresentacao

Uma explicacao simples:

> O programa usa `mmap` para acessar o arquivo grande em memoria. Depois ele
> divide o arquivo em blocos alinhados por linha. Cada thread processa um bloco
> independente e gera uma hash table local com contagens por IP. Quando todas as
> threads terminam, o programa faz o reduce juntando as tabelas locais em uma
> tabela global. Por fim, ordena essa tabela para mostrar os IPs com mais falhas.

## Onde esta cada conceito

- Entrada do programa: `src/main.c`
- Orquestracao: `src/auth_analyzer.c`
- Split: `src/split.c`
- Map: `src/map.c`
- Reduce/Merge: `src/hashtable.c`, funcao `ht_merge`
- Contagem local: `src/hashtable.c`, funcao `ht_increment`
- Parser de log: `src/utils/log_parser.c`
- Saida final: `src/utils/report.c`
- Medicao de tempo: `src/utils/timer_utils.c`
- Leitura eficiente do arquivo: `src/utils/mmap_utils.c`
