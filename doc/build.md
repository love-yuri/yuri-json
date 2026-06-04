# 构建与测试

## 工具链要求

- GCC 16+（需要 `-freflection`）
- CMake 3.30+
- Ninja
- x86-64 平台

## 一键脚本

克隆后直接运行：

```bash
# 单元测试（108 个用例，无需额外依赖）
./script/test.sh

# 性能 benchmark
./script/benchmark.sh
```

两个脚本各自使用独立的构建目录（`build-test/` / `build-bench/`），互不干扰。

## Benchmark 依赖

benchmark 需要安装以下库（Arch Linux）：

```bash
sudo pacman -S nlohmann-json rapidjson glaze simdjson
```

单元测试不需要任何外部依赖。

## 手动构建

### Demo

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -Wno-dev
cmake --build build
./build/json_demo
```

### 单元测试

```bash
cmake -S . -B build-test -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON -Wno-dev
cmake --build build-test -j$(nproc)
ctest --test-dir build-test --output-on-failure
```

### Benchmark

```bash
cmake -S . -B build-bench -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_BENCHMARK=ON -Wno-dev
cmake --build build-bench -j$(nproc)
ctest --test-dir build-bench --output-on-failure
./build-bench/benchmark/bench_yuri-json
```

## 测试套件

| 可执行文件 | 用例数 | 说明 |
|-----------|--------|------|
| `test_serialize` | 12 | 序列化：基础类型、转义、容器、中文 |
| `test_deserialize` | 12 | 反序列化：正确解析、乱序key、大数组 |
| `test_roundtrip` | 7 | 序列化→反序列化一致性 |
| `test_malformed` | 23 | JSON 结构错误：截断、缺引号/冒号/逗号、尾逗号 |
| `test_field_mismatch` | 21 | 字段不匹配：多余/缺失字段、类型不匹配 |
| `test_diagnostics` | 15 | 诊断输出：行号、列号、snippet、caret |
| `test_edge_cases` | 18 | 边界：INT_MAX、超长字符串、Unicode、大数组 |
