
FILE=${1:-data/auth_5M.log}

if [ ! -f "$FILE" ]; then
    echo "Arquivo nao encontrado: $FILE"
    exit 1
fi

echo "========================================"
echo " Analisador Paralelo de Auth Logs"
echo "========================================"
echo "Arquivo: $FILE"
echo ""

T1=$(./authanalyzer "$FILE" 1 2>/dev/null | grep "Total" | awk '{print $3}' | tr -d 's')
echo "1 thread : ${T1}s  (baseline sequencial)"

for t in 2 4 8; do
    Tp=$(./authanalyzer "$FILE" $t 2>/dev/null | grep "Total" | awk '{print $3}' | tr -d 's')
    SP=$(awk "BEGIN {printf \"%.2f\", $T1 / $Tp}")
    EF=$(awk "BEGIN {printf \"%.0f\", ($T1 / $Tp) / $t * 100}")
    echo "$t threads: ${Tp}s  |  Speedup: ${SP}x  |  Eficiencia: ${EF}%"
done

echo ""
echo "=== Relatorio de seguranca (8 threads) ==="
./authanalyzer "$FILE" 8 2>/dev/null | grep -A 12 "Top 10"