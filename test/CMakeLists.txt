# ---- yuri-json 单元测试 ----
set(TEST_SOURCES
  test_serialize
  test_deserialize
  test_roundtrip
  test_malformed
  test_field_mismatch
  test_diagnostics
  test_edge_cases
)

foreach(src ${TEST_SOURCES})
  add_executable(${src} ${src}.cpp)
  target_link_libraries(${src} PRIVATE yuri_json)
  target_include_directories(${src} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
  add_test(NAME ${src} COMMAND ${src})
endforeach()
