#!/bin/bash
###
 # @Author: love-yuri yuri2078170658@gmail.com
 # @Date: 2026-06-01 16:46:23
 # @LastEditTime: 2026-06-01 16:50:36
 # @Description: 
### 
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

LIBS=("yuri-json" "nlohmann" "glaze" "rapidjson" "simdjson")
BINS=("build/test/bench_yuri-json" "build/test/bench_nlohmann" "build/test/bench_glaze" "build/test/bench_rapidjson" "build/test/bench_simdjson")
declare -A R

for i in "${!LIBS[@]}"; do
    lib="${LIBS[$i]}"
    bin="${BINS[$i]}"
    while IFS= read -r line; do
        [[ "$line" =~ SmallObj[[:space:]]+([0-9.]+)[[:space:]]*us[[:space:]]+([0-9.]+)[[:space:]]*us ]] && R["${lib}_SmallObj_s"]="${BASH_REMATCH[1]}" && R["${lib}_SmallObj_d"]="${BASH_REMATCH[2]}"
        [[ "$line" =~ MediumObj[[:space:]]+([0-9.]+)[[:space:]]*us[[:space:]]+([0-9.]+)[[:space:]]*us ]] && R["${lib}_MediumObj_s"]="${BASH_REMATCH[1]}" && R["${lib}_MediumObj_d"]="${BASH_REMATCH[2]}"
        [[ "$line" =~ ComplexObj[[:space:]]+([0-9.]+)[[:space:]]*us[[:space:]]+([0-9.]+)[[:space:]]*us ]] && R["${lib}_ComplexObj_s"]="${BASH_REMATCH[1]}" && R["${lib}_ComplexObj_d"]="${BASH_REMATCH[2]}"
        [[ "$line" =~ LargeObj.*[[:space:]]+([0-9.]+)[[:space:]]*us[[:space:]]+([0-9.]+)[[:space:]]*us ]] && R["${lib}_LargeObj_s"]="${BASH_REMATCH[1]}" && R["${lib}_LargeObj_d"]="${BASH_REMATCH[2]}"
        [[ "$line" =~ LargeObj.*N/A[[:space:]]+([0-9.]+)[[:space:]]*us ]] && R["${lib}_LargeObj_s"]="N/A" && R["${lib}_LargeObj_d"]="${BASH_REMATCH[1]}"
    done < <("$bin" 2>&1)
done

fmt() { [[ "$1" == "N/A" ]] && echo "N/A" || echo "$1 us"; }

echo ""
echo "  ┌───────────┬──────────────┬────────────────┬───────────────┬─────────────────┬────────────────┬──────────────────┬────────────────┬──────────────────┐"
echo "  │     库    │ SmallObj序列 │ SmallObj反序列 │ MediumObj序列 │ MediumObj反序列 │ ComplexObj序列 │ ComplexObj反序列 │ LargeObj序列化 │ LargeObj反序列化 │"
echo "  │           │      化      │       化       │      化       │       化        │       化       │        化        │    (~15KB)     │     (~15KB)      │"
echo "  ├───────────┼──────────────┼────────────────┼───────────────┼─────────────────┼────────────────┼──────────────────┼────────────────┼──────────────────┤"
first=true
for lib in "${LIBS[@]}"; do
    $first && first=false || echo "  ├───────────┼──────────────┼────────────────┼───────────────┼─────────────────┼────────────────┼──────────────────┼────────────────┼──────────────────┤"
    printf "  │ %-9s │ %-12s │ %-14s │ %-13s │ %-15s │ %-14s │ %-16s │ %-14s │ %-16s │\n" "$lib" \
        "$(fmt "${R[${lib}_SmallObj_s]:-N/A}")" \
        "$(fmt "${R[${lib}_SmallObj_d]:-N/A}")" \
        "$(fmt "${R[${lib}_MediumObj_s]:-N/A}")" \
        "$(fmt "${R[${lib}_MediumObj_d]:-N/A}")" \
        "$(fmt "${R[${lib}_ComplexObj_s]:-N/A}")" \
        "$(fmt "${R[${lib}_ComplexObj_d]:-N/A}")" \
        "$(fmt "${R[${lib}_LargeObj_s]:-N/A}")" \
        "$(fmt "${R[${lib}_LargeObj_d]:-N/A}")"
done
echo "  └───────────┴──────────────┴────────────────┴───────────────┴─────────────────┴────────────────┴──────────────────┴────────────────┴──────────────────┘"
