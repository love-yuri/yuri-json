#!/bin/bash
set -e

cd "$(dirname "$0")/.."

echo "============================================"
echo "  JSON库性能对比测试"
echo "============================================"

# 配置 + 构建
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON -Wno-dev
cmake --build build -j$(nproc)

# ctest
echo ""
echo "--- CTest ---"
cd build && ctest --output-on-failure
cd ..

# benchmark对比表
echo ""
echo "--- Benchmark ---"

LIBS=("reflect" "nlohmann" "glaze" "rapidjson" "simdjson")
BINS=("build/test/bench_reflect" "build/test/bench_nlohmann" "build/test/bench_glaze" "build/test/bench_rapidjson" "build/test/bench_simdjson")
declare -A R

for i in "${!LIBS[@]}"; do
    lib="${LIBS[$i]}"
    bin="${BINS[$i]}"
    while IFS= read -r line; do
        [[ "$line" =~ ^[[:space:]]*([A-Za-z]+)[[:space:]]+([0-9.]+)[[:space:]]*us[[:space:]]+([0-9.]+)[[:space:]]*us ]] && R["${lib}_${BASH_REMATCH[1]}_s"]="${BASH_REMATCH[2]}" && R["${lib}_${BASH_REMATCH[1]}_d"]="${BASH_REMATCH[3]}"
        [[ "$line" =~ ^[[:space:]]*([A-Za-z]+)[[:space:]]+N/A[[:space:]]+([0-9.]+)[[:space:]]*us ]] && R["${lib}_${BASH_REMATCH[1]}_s"]="N/A" && R["${lib}_${BASH_REMATCH[1]}_d"]="${BASH_REMATCH[2]}"
    done < <("$bin" 2>&1)
done

fmt() { [[ "$1" == "N/A" ]] && echo "N/A" || echo "$1 us"; }

echo ""
echo "  ┌───────────┬──────────────┬────────────────┬───────────────┬─────────────────┬────────────────┬──────────────────┐"
echo "  │     库    │ SmallObj序列 │ SmallObj反序列 │ MediumObj序列 │ MediumObj反序列 │ ComplexObj序列 │ ComplexObj反序列 │"
echo "  │           │      化      │       化       │      化       │       化        │       化       │        化        │"
echo "  ├───────────┼──────────────┼────────────────┼───────────────┼─────────────────┼────────────────┼──────────────────┤"
first=true
for lib in "${LIBS[@]}"; do
    $first && first=false || echo "  ├───────────┼──────────────┼────────────────┼───────────────┼─────────────────┼────────────────┼──────────────────┤"
    printf "  │ %-9s │ %-12s │ %-14s │ %-13s │ %-15s │ %-14s │ %-16s │\n" "$lib" \
        "$(fmt "${R[${lib}_SmallObj_s]:-N/A}")" \
        "$(fmt "${R[${lib}_SmallObj_d]:-N/A}")" \
        "$(fmt "${R[${lib}_MediumObj_s]:-N/A}")" \
        "$(fmt "${R[${lib}_MediumObj_d]:-N/A}")" \
        "$(fmt "${R[${lib}_ComplexObj_s]:-N/A}")" \
        "$(fmt "${R[${lib}_ComplexObj_d]:-N/A}")"
done
echo "  └───────────┴──────────────┴────────────────┴───────────────┴─────────────────┴────────────────┴──────────────────┘"
