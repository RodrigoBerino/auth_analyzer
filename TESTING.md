# Testes do Auth Analyzer

Este arquivo mostra como testar o projeto depois da reorganizacao em arquivos
principais e `src/utils/`.

O objetivo dos testes e validar tres coisas:

1. o projeto compila sem warnings;
2. o resultado do Top IPs esta correto;
3. o resultado nao muda quando o numero de threads muda.

## 1. Compilar o projeto

Rode:

```bash
make clean && make
```

Resultado esperado:

- o binario `authanalyzer` deve ser criado;
- nao deve aparecer warning do compilador;
- o comando deve terminar sem erro.

## 2. Rodar um teste funcional simples

Rode com 4 threads:

```bash
./authanalyzer data/auth_1M.log 4
```

Resultado esperado:

- deve aparecer o bloco `Relatorio de autenticacao`;
- deve aparecer a tabela `Top 10 IPs com mais falhas`;
- deve aparecer o bloco `Desempenho`;
- o programa deve terminar sem erro.

Exemplo de formato esperado:

```text
=== Relatorio de autenticacao ===

Top 10 IPs com mais falhas
#    IP                     Falhas
---  ------------------ ----------
1    103.99.0.118           133353
...

Desempenho
Arquivo    : data/auth_1M.log
Threads    : 4
Linhas     : 1000000
Split      : ...
Map        : ...
Merge      : ...
Total      : ...
Throughput : ...
```

## 3. Comparar com o gabarito

O arquivo `data/gabarito_top10_auth_1M.txt` tem o resultado esperado gerado por
comandos de shell.

Veja o gabarito:

```bash
cat data/gabarito_top10_auth_1M.txt
```

Rode o programa:

```bash
./authanalyzer data/auth_1M.log 1
```

Compare os IPs e as contagens do Top IPs.

Resultado esperado para `data/auth_1M.log`:

```text
1    103.99.0.118           133353
2    178.128.88.200         133276
3    45.33.32.156           133246
4    91.240.118.50          132514
5    185.220.101.42         132249
6    192.168.1.20             3873
7    10.0.0.45                3783
8    10.0.0.33                3752
9    192.168.1.10             3650
```

## 4. Verificar se o resultado nao muda com mais threads

Este teste valida que o split, o map paralelo e o merge estao corretos.

```bash
./authanalyzer data/auth_1M.log 1 | sed -n '5,15p' > /tmp/auth_top_1.txt
./authanalyzer data/auth_1M.log 4 | sed -n '5,15p' > /tmp/auth_top_4.txt
./authanalyzer data/auth_1M.log 8 | sed -n '5,15p' > /tmp/auth_top_8.txt

diff /tmp/auth_top_1.txt /tmp/auth_top_4.txt && echo "1 == 4 threads OK"
diff /tmp/auth_top_1.txt /tmp/auth_top_8.txt && echo "1 == 8 threads OK"
```

Resultado esperado:

```text
1 == 4 threads OK
1 == 8 threads OK
```

Se o `diff` mostrar diferencas, alguma parte do processamento paralelo esta
errada.

## 5. Salvar a saida em arquivo

Se quiser olhar o resultado fora do terminal:

```bash
./authanalyzer data/auth_1M.log 4 > resultado.txt
```

Depois abra o arquivo:

```bash
less resultado.txt
```

Ou:

```bash
cat resultado.txt
```

## 6. Testar argumentos invalidos

Sem argumentos:

```bash
./authanalyzer
```

Resultado esperado:

```text
Uso: ./authanalyzer <arquivo.log> <n_threads>
```

Com numero invalido de threads:

```bash
./authanalyzer data/auth_1M.log 0
```

Resultado esperado:

```text
Erro: <n_threads> deve ser maior que zero.
```

Com arquivo inexistente:

```bash
./authanalyzer data/nao_existe.log 4
```

Resultado esperado:

```text
open: No such file or directory
```

## 7. Testar desempenho com dataset maior

Rode:

```bash
bash scripts/demo_speedup.sh data/auth_5M.log
```

Este script deve rodar o programa com diferentes numeros de threads.

Use esse teste para observar:

- tempo total;
- throughput;
- se o Top IPs continua igual;
- ganho de desempenho com mais threads.

## 8. Gerar novo dataset de teste

Se quiser gerar outro arquivo de logs:

```bash
python3 scripts/generate_auth_logs.py 100000 data/auth_100k.log
./authanalyzer data/auth_100k.log 4
```

Use um numero menor de linhas quando quiser testar rapido.

## 9. Conferir vazamento de memoria com Valgrind

Opcional. Se o Valgrind estiver instalado:

```bash
valgrind --leak-check=full ./authanalyzer data/auth_1M.log 4
```

Resultado ideal no final:

```text
All heap blocks were freed -- no leaks are possible
```

Se nao tiver Valgrind instalado, essa etapa pode ser ignorada.

## 10. Roteiro recomendado antes de apresentar

Rode esta sequencia:

```bash
make clean && make
./authanalyzer data/auth_1M.log 1
./authanalyzer data/auth_1M.log 4
./authanalyzer data/auth_1M.log 8
bash scripts/demo_speedup.sh data/auth_5M.log
```

O que verificar:

- o Top IPs e igual em 1, 4 e 8 threads;
- o tempo total tende a cair quando usa mais threads;
- o bloco `Map` deve ser a parte mais importante do tempo;
- a saida final esta legivel para explicar.
