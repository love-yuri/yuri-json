# yuri_json

基于 C++26 静态反射的高性能 JSON 序列化/反序列化库。

## 特性

- **零注册**：定义 `struct` 即可使用，无需宏或特化
- **C++26 反射**：通过 `std::meta` 自动遍历结构体成员
- **SSE2 加速**：空白跳过、字符串扫描使用 SIMD 并行处理
- **x64 only**：仅支持 x86-64 平台，GCC 编译器

## 环境要求

- GCC 16+（支持 `-freflection`）
- CMake 3.30+
- Ninja

## 快速开始

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -Wno-dev
cmake --build build
./build/json_demo
```

## 使用

```cpp
import yuri.json;

struct Person {
    std::string name;
    int age;
    double height;
    bool active;
};

int main() {
    Person p{"Alice", 30, 1.65, true};
    std::string s = yuri::to_json(p);
    Person p2 = yuri::from_json<Person>(s);
}
```

支持的类型：`std::string`、`int`、`long long`、`double`、`float`、`bool`、`std::vector<T>`（T 为上述类型）。

## 在项目中使用

### 方式一：作为子目录

将本仓库克隆到你的项目中：

```bash
git clone https://github.com/love-yuri/yuri-json libs/yuri_json
```

在你的 `CMakeLists.txt` 中：

```cmake
cmake_minimum_required(VERSION 3.30)
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "d0edc3af-4c50-42ea-a356-e2862fe7a444")
project(your_project LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_MODULE_STD 1)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -freflection")

add_subdirectory(libs/yuri_json)

add_executable(your_app main.cpp)
target_link_libraries(your_app PRIVATE yuri_json)
```

在代码中：

```cpp
import yuri.json;

struct Config {
   std::string name;
  int timeout;
  bool verbose;
};

void save(const Config &cfg) {
  std::string json = yuri::to_json(cfg);
  // ...
}

Config load(std::string_view json) {
  return yuri::from_json<Config>(json);
}
```

### 方式二：编译后链接

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -Wno-dev
cmake --build build
```

在你的 `CMakeLists.txt` 中：

```cmake
# 指定 yuri_json 的路径
set(YURI_JSON_DIR "/path/to/yuri_json")

add_library(yuri_json STATIC IMPORTED)
set_target_properties(yuri_json PROPERTIES
  IMPORTED_LOCATION "${YURI_JSON_DIR}/build/libyuri_json.a"
  INTERFACE_INCLUDE_DIRECTORIES "${YURI_JSON_DIR}/src"
)

# 需要与 yuri_json 相同的编译标志
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -freflection")

add_executable(your_app main.cpp)
target_link_libraries(your_app PRIVATE yuri_json)
```

> **注意**：使用本库的项目必须满足相同的环境要求（GCC 16+、CMake 3.30+、C++26、`-freflection`）。

## Benchmark

```bash
# 安装依赖（Arch Linux）
sudo pacman -S nlohmann-json rapidjson glaze simdjson

# 运行 benchmark
bash script/benchmark.sh
```

输出示例：

```
  ┌───────────┬──────────────┬────────────────┬───────────────┬─────────────────┬────────────────┬──────────────────┐
  │     库    │ SmallObj序列 │ SmallObj反序列 │ MediumObj序列 │ MediumObj反序列 │ ComplexObj序列 │ ComplexObj反序列 │
  │           │      化      │       化       │      化       │       化        │       化       │        化        │
  ├───────────┼──────────────┼────────────────┼───────────────┼─────────────────┼────────────────┼──────────────────┤
  │ reflect   │ 0.092 us     │ 0.125 us       │ 0.190 us      │ 0.398 us        │ 0.248 us       │ 0.540 us         │
  ├───────────┼──────────────┼────────────────┼───────────────┼─────────────────┼────────────────┼──────────────────┤
  │ glaze     │ 0.068 us     │ 0.028 us       │ 0.187 us      │ 0.180 us        │ 0.465 us       │ 0.303 us         │
  ├───────────┼──────────────┼────────────────┼───────────────┼─────────────────┼────────────────┼──────────────────┤
  │ rapidjson │ 0.108 us     │ 0.125 us       │ 0.255 us      │ 0.388 us        │ 0.546 us       │ 0.483 us         │
  └───────────┴──────────────┴────────────────┴───────────────┴─────────────────┴────────────────┴──────────────────┘
```

仅运行 CTest 验证：

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON -Wno-dev
cmake --build build
cd build && ctest --output-on-failure
```

## 项目结构

```
├── CMakeLists.txt          # 主构建配置
├── main.cpp                # 使用示例
├── src/
│   ├── json.cppm           # 主模块接口
│   ├── simd.cppm           # SSE2 SIMD 基础封装
│   ├── string_utils.cppm   # SIMD 加速字符串扫描
│   ├── output.cppm         # 数值/字符串/数组序列化
│   ├── parser.cppm         # JSON 解析器
│   └── reflection.cppm     # 反射序列化/反序列化
├── script/
│   └── benchmark.sh        # benchmark 脚本
└── test/
    ├── CMakeLists.txt       # 测试构建配置
    ├── bench_common.hpp     # 共享测试数据结构
    ├── bench_reflect.cpp    # 本库 benchmark
    ├── bench_nlohmann.cpp   # nlohmann/json benchmark
    ├── bench_glaze.cpp      # glaze benchmark
    ├── bench_rapidjson.cpp  # rapidjson benchmark
    └── bench_simdjson.cpp   # simdjson benchmark
```
