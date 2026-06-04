#!/bin/bash
###
 # @Author: love-yuri yuri2078170658@gmail.com
 # @Date: 2026-06-04
 # @Description: 一键运行 yuri-json 全部单元测试（不包含 benchmark）
###
set -e

cd "$(dirname "$0")/.."

BUILD_DIR="build-test"

echo ""
echo "  ╔══════════════════════════════════════════╗"
echo "  ║       yuri-json 单元测试                 ║"
echo "  ╚══════════════════════════════════════════╝"
echo ""

# 配置 + 构建（仅测试，不含 benchmark）
echo ">>> 配置 + 构建..."
cmake -S . -B "$BUILD_DIR" -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON -DBUILD_BENCHMARK=OFF -Wno-dev
cmake --build "$BUILD_DIR" -j$(nproc)

echo ""
echo ">>> 运行测试..."
echo ""

# 测试列表
TESTS=(
  "test_serialize:序列化测试"
  "test_deserialize:反序列化测试"
  "test_roundtrip:Round-trip 测试"
  "test_malformed:JSON 结构错误测试"
  "test_field_mismatch:字段不匹配测试"
  "test_diagnostics:诊断输出能力验证"
  "test_edge_cases:边界情况测试"
)

TOTAL_PASS=0
TOTAL_FAIL=0
FAILED_LIST=()

for entry in "${TESTS[@]}"; do
  name="${entry%%:*}"
  bin="${BUILD_DIR}/test/${name}"

  if [[ ! -x "$bin" ]]; then
    echo "  ✗ ${name} (未找到可执行文件)"
    FAILED_LIST+=("$name")
    ((TOTAL_FAIL++))
    continue
  fi

  output=$("$bin" 2>&1)
  exit_code=$?

  # 显示完整输出
  echo "$output"

  if [[ $exit_code -eq 0 ]]; then
    # 提取通过/失败数
    pass=$(echo "$output" | grep -oP '(?<=结果: )\d+' | head -1)
    fail=$(echo "$output" | grep -oP '(?<=, )\d+(?= 失败)' | head -1)
    TOTAL_PASS=$((TOTAL_PASS + ${pass:-0}))
    TOTAL_FAIL=$((TOTAL_FAIL + ${fail:-0}))
  else
    echo "  ✗ ${name} 异常退出 (code=${exit_code})"
    FAILED_LIST+=("$name")
    TOTAL_FAIL=$((TOTAL_FAIL + 1))
  fi
done

# 汇总
echo ""
echo "  ┌──────────────────────────────────────────┐"
printf "  │  %-36s │\n" "总计: ${TOTAL_PASS} 通过, ${TOTAL_FAIL} 失败"
if [[ ${#FAILED_LIST[@]} -gt 0 ]]; then
  printf "  │  %-36s │\n" "失败: ${FAILED_LIST[*]}"
fi
echo "  └──────────────────────────────────────────┘"
echo ""

# CTest
echo ">>> CTest 汇总:"
ctest --test-dir "$BUILD_DIR" --output-on-failure 2>&1 | tail -5
echo ""

[[ $TOTAL_FAIL -eq 0 ]] && exit 0 || exit 1
