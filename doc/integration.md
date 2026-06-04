# 在项目中使用

## 方式一：作为子目录

将仓库放入你的项目中：

```bash
git clone https://github.com/love-yuri/yuri-json libs/yuri_json
```

在你的 `CMakeLists.txt` 中：

```cmake
cmake_minimum_required(VERSION 3.30)
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "451f2fe2-a8a2-47c3-bc32-94786d8fc91b")
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

代码示例：

```cpp
import yuri.json;

struct Config {
  std::string name;
  int timeout;
  bool verbose;
};

void save(const Config &cfg) {
  std::string json = yuri::to_json(cfg);
}

Config load(std::string_view json) {
  return yuri::from_json<Config>(json);
}
```

## 方式二：编译后链接

先构建 `yuri_json`：

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -Wno-dev
cmake --build build
```

在你的 `CMakeLists.txt` 中：

```cmake
set(YURI_JSON_DIR "/path/to/yuri_json")

add_library(yuri_json STATIC IMPORTED)
set_target_properties(yuri_json PROPERTIES
  IMPORTED_LOCATION "${YURI_JSON_DIR}/build/libyuri_json.a"
  INTERFACE_INCLUDE_DIRECTORIES "${YURI_JSON_DIR}/src"
)
```

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -freflection")

add_executable(your_app main.cpp)
target_link_libraries(your_app PRIVATE yuri_json)
```

使用本库的项目必须满足相同工具链要求：GCC 16+、CMake 3.30+、C++26、`-freflection`。
