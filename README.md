## authAnalyzer

Analisador paralelo de logs SSH escrito em C.

O programa lê um arquivo `auth.log`, divide o conteúdo em blocos, processa os
blocos com `pthread`, conta falhas de autenticação por IP e imprime um relatório
com os IPs mais recorrentes.

## Compilação

```bash
make
```

Para limpar os arquivos gerados:

```bash
make clean
```

## Execução

Formato:

```bash
./authanalyzer <arquivo.log> <n_threads>
```

Exemplo usando o arquivo de 5 milhões de linhas:

```bash
./authanalyzer data/auth_5M.log 4
```

O segundo argumento define a quantidade de threads. Exemplo com 8 threads:

```bash
./authanalyzer data/auth_5M.log 8
```

## Fluxo do programa

```text
main.c
  -> auth_analyzer.c
      -> mmap_utils.c   mapeia o arquivo em memória
      -> split.c        divide o arquivo em blocos
      -> map.c          processa os blocos em threads
      -> log_parser.c   identifica falhas e extrai IPs
      -> hashtable.c    conta e combina ocorrências
      -> report.c       imprime o relatório final
```

## Estrutura principal

- `src/main.c`: entrada do programa e validação dos argumentos.
- `src/auth_analyzer.c`: orquestra o fluxo principal.
- `src/split.c`: divide o arquivo sem cortar linhas no meio.
- `src/map.c`: função executada por cada thread.
- `src/hashtable.c`: tabela hash usada para contar IPs.
- `src/utils/mmap_utils.c`: leitura do arquivo com `mmap`.
- `src/utils/log_parser.c`: identificação de falhas e extração de IP.
- `src/utils/report.c`: impressão do resultado.

## Teste básico

```bash
make clean && make
./authanalyzer data/auth_5M.log 1
./authanalyzer data/auth_5M.log 4
./authanalyzer data/auth_5M.log 8
```

As contagens finais devem ser equivalentes para qualquer quantidade de threads.
