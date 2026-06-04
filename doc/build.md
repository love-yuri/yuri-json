# 构建与测试

## 工具链要求

- GCC 16+（需要 `-freflection`）
- CMake 3.30+
- Ninja
- x86-64 平台

## 构建 demo

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -Wno-dev
cmake --build build
./build/json_demo
```

## 单元测试

一键运行全部 108 个测试用例：

```bash
bash script/test.sh
```

或手动构建运行：

```bash
cmake -S . -B build-test -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON -Wno-dev
cmake --build build-test -j$(nproc)
ctest --test-dir build-test --output-on-failure
```

测试构建目录：`build-test/`，与 benchmark 独立，互不干扰。

### 测试套件

| 可执行文件 | 用例数 | 说明 |
|-----------|--------|------|
| `test_serialize` | 12 | 序列化：基础类型、转义、容器、中文 |
| `test_deserialize` | 12 | 反序列化：正确解析、乱序key、大数组 |
| `test_roundtrip` | 7 | 序列化→反序列化一致性 |
| `test_malformed` | 23 | JSON 结构错误：截断、缺引号/冒号/逗号、尾逗号 |
| `test_field_mismatch` | 21 | 字段不匹配：多余/缺失字段、类型不匹配 |
| `test_diagnostics` | 15 | 诊断输出：行号、列号、snippet、caret |
| `test_edge_cases` | 18 | 边界：INT_MAX、超长字符串、Unicode、大数组 |

## Benchmark

安装可选依赖（Arch Linux）：

```bash
sudo pacman -S nlohmann-json rapidjson glaze simdjson
```

一键运行性能对比：

```bash
bash script/benchmark.sh
```

或手动构建运行：

```bash
cmake -S . -B build-bench -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_BENCHMARK=ON -Wno-dev
cmake --build build-bench -j$(nproc)
ctest --test-dir build-bench --output-on-failure
./build-benchmark/benchmark/bench_yuri-json
```

Benchmark 构建目录：`build-bench/`，与测试独立，互不干扰。
