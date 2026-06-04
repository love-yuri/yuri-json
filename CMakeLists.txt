cmake_minimum_required(VERSION 3.30)
cmake_policy(SET CMP0149 NEW)
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "451f2fe2-a8a2-47c3-bc32-94786d8fc91b")
project(yuri_json LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_MODULE_STD 1)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -freflection")

# ---- 主库 ----
add_library(yuri_json STATIC)
target_sources(yuri_json PUBLIC FILE_SET CXX_MODULES FILES
  src/json.cppm
  src/simd.cppm
  src/string_utils.cppm
  src/output.cppm
  src/parser.cppm
  src/reflection.cppm
)

add_executable(json_demo main.cpp)
target_link_libraries(json_demo PRIVATE yuri_json)

# ---- 测试 ----
option(BUILD_TESTING "Build unit tests" OFF)
if(BUILD_TESTING)
  enable_testing()
  add_subdirectory(test)
endif()

# ---- Benchmark ----
option(BUILD_BENCHMARK "Build benchmark tests" OFF)
if(BUILD_BENCHMARK)
  enable_testing()
  add_subdirectory(benchmark)
endif()
