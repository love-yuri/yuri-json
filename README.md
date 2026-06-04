# yuri-json

基于 C++26 静态反射的高性能 JSON 序列化/反序列化库。

## 项目特色

- **零注册**：定义 `struct` 即可序列化和反序列化，无需宏或手写特化。
- **静态反射**：基于 C++26 `std::meta` 自动遍历结构体字段。
- **严格解析**：默认解析失败直接抛异常，可选彩色诊断输出行列、片段和期望内容。
- **SIMD 加速**：SSE2 加速字符串扫描、空白跳过和转义检测。
- **轻量模块**：C++ modules only, `import yuri.json;` 即可使用。

## 快速使用

```cpp
import std;
import yuri.json;

struct Person {
  std::string name;   // 姓名
  int age;            // 年龄
  double height;      // 身高
  bool active;        // 是否激活
  std::vector<std::string> hobbies; // 爱好
};

int main() {
  // 序列化
  Person p{ "Alice", 30, 1.65, true, { "reading", "coding" } };
  std::string json = yuri::to_json(p);

  // 反序列化
  Person copy = yuri::from_json<Person>(json);

  // 带诊断的反序列化（错误时输出彩色行列信息）
  try {
    Person bad = yuri::from_json<Person>(invalid_json, yuri::kWithDiagnostics);
  } catch (const yuri::JsonParseError &e) {
    std::println("{}", e.what()); // 彩色错误信息
  }
}
```

支持类型：`std::string`、`int`、`long long`、`double`、`float`、`bool`、`std::vector<T>`。

## 工具链

- GCC 16+（需要 `-freflection`）
- CMake 3.30+
- Ninja
- x86-64 平台

## 构建与运行

```bash
# 构建 demo
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -Wno-dev
cmake --build build
./build/json_demo

# 一键运行单元测试（108 个用例）
bash script/test.sh

# 一键运行性能 benchmark
bash script/benchmark.sh
```

## 测试

单元测试覆盖 7 大类 108 个用例：

| 测试套件 | 用例数 | 覆盖内容 |
|---------|--------|---------|
| `test_serialize` | 12 | 序列化：基础类型、转义、容器、中文 |
| `test_deserialize` | 12 | 反序列化：正确解析、乱序key、大数组 |
| `test_roundtrip` | 7 | 序列化→反序列化一致性 |
| `test_malformed` | 23 | JSON 结构错误：截断、缺引号/冒号/逗号、尾逗号 |
| `test_field_mismatch` | 21 | 字段不匹配：多余/缺失字段、类型不匹配 |
| `test_diagnostics` | 15 | 诊断输出：行号、列号、snippet、caret |
| `test_edge_cases` | 18 | 边界：INT_MAX、超长字符串、Unicode、大数组 |

## Benchmark

单位：微秒/次，数值越低越好。

| 库 | Small 序列化 | Small 反序列化 | Medium 序列化 | Medium 反序列化 | Large 序列化 | Large 反序列化 |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| yuri-json | 0.069 | 0.042 | 0.173 | 0.161 | 9.777 | 5.269 |
| glaze | 0.059 | 0.027 | 0.171 | 0.173 | 10.501 | 7.087 |
| rapidjson | 0.107 | 0.113 | 0.251 | 0.403 | 14.684 | 9.805 |
| nlohmann | 0.298 | 0.532 | 0.823 | 1.419 | 29.675 | 65.173 |

完整 benchmark 运行方式见 [构建与测试](doc/build.md)。

## 文档

- [构建与测试](doc/build.md)
- [在项目中使用](doc/integration.md)
