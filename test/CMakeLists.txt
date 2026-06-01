# ---- 依赖（Arch Linux: pacman -S nlohmann-json rapidjson glaze simdjson） ----
find_package(nlohmann_json REQUIRED)
find_package(glaze REQUIRED)
find_package(simdjson REQUIRED)
find_path(RAPIDJSON_INCLUDE_DIR rapidjson/document.h REQUIRED)

# ---- json反射benchmark ----
add_executable(bench_reflect bench_reflect.cpp)
target_link_libraries(bench_reflect PRIVATE yuri_json)
target_include_directories(bench_reflect PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
add_test(NAME bench_reflect COMMAND bench_reflect)

# ---- nlohmann benchmark ----
add_executable(bench_nlohmann bench_nlohmann.cpp)
target_link_libraries(bench_nlohmann PRIVATE nlohmann_json::nlohmann_json)
target_include_directories(bench_nlohmann PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(bench_nlohmann PRIVATE -Wno-template-body)
add_test(NAME bench_nlohmann COMMAND bench_nlohmann)

# ---- glaze benchmark ----
add_executable(bench_glaze bench_glaze.cpp)
target_link_libraries(bench_glaze PRIVATE glaze::glaze)
target_include_directories(bench_glaze PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(bench_glaze PRIVATE -Wno-template-body)
add_test(NAME bench_glaze COMMAND bench_glaze)

# ---- rapidjson benchmark ----
add_executable(bench_rapidjson bench_rapidjson.cpp)
target_include_directories(bench_rapidjson PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${RAPIDJSON_INCLUDE_DIR}
)
target_compile_options(bench_rapidjson PRIVATE
  -Wno-deprecated-declarations
  -Wno-template-body
)
add_test(NAME bench_rapidjson COMMAND bench_rapidjson)

# ---- simdjson benchmark ----
add_executable(bench_simdjson bench_simdjson.cpp)
target_link_libraries(bench_simdjson PRIVATE simdjson::simdjson)
target_include_directories(bench_simdjson PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(bench_simdjson PRIVATE -Wno-template-body)
add_test(NAME bench_simdjson COMMAND bench_simdjson)
