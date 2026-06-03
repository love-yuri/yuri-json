# yuri-json

基于 C++26 静态反射的高性能 JSON 序列化/反序列化库。

## 项目特色

- **零注册**：定义 `struct` 即可序列化和反序列化，无需宏或手写特化。
- **静态反射**：基于 C++26 `std::meta` 自动遍历结构体字段。
- **严格解析**：默认解析失败直接抛异常，可选彩色诊断输出行列、片段和期望内容。
- **轻量模块**：C++ modules only, `import yuri.json;` 即可使用。

## 快速使用

```cpp
import std;
import yuri.json;

struct Person {
  std::string name;
  int age;
  double height;
  bool active;
};

int main() {
  Person p{ "Alice", 30, 1.65, true };
  std::string json = yuri::to_json(p);
  Person copy = yuri::from_json<Person>(json);
}
```

支持类型：`std::string`、`int`、`long long`、`double`、`float`、`bool`、`std::vector<T>`。

## 工具链

- GCC 16+
- CMake 3.30+
- Ninja

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

## 运行 demo

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -Wno-dev
cmake --build build
./build/json_demo
```
