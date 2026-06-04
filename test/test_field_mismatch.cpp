#include "test_common.hpp"

// ============================================================================
// 字段不匹配测试：JSON 与 struct 字段的对应关系
// ============================================================================

// --- JSON 有字段、struct 没有（多余字段） ---

TEST_CASE(extra_single_field) {
  auto obj = yuri::from_json<SmallObj>(R"({
    "name": "Test",
    "id": 1,
    "score": 1.0,
    "active": true,
    "extra_field": 42
  })");
  EXPECT_EQ(obj.name, "Test");
  printPass("单个多余字段");
}

TEST_CASE(extra_multiple_fields) {
  auto obj = yuri::from_json<SmallObj>(R"({
    "name": "Test",
    "id": 1,
    "score": 1.0,
    "active": true,
    "aaa": 1,
    "bbb": "two",
    "ccc": true,
    "ddd": 3.14
  })");
  EXPECT_EQ(obj.name, "Test");
  printPass("多个多余字段");
}

TEST_CASE(extra_nested_object) {
  auto obj = yuri::from_json<SmallObj>(R"({
    "name": "Test",
    "id": 1,
    "score": 1.0,
    "active": true,
    "nested": {"deep": "value"}
  })");
  EXPECT_EQ(obj.name, "Test");
  printPass("多余嵌套对象");
}

TEST_CASE(extra_nested_array) {
  auto obj = yuri::from_json<SmallObj>(R"({
    "name": "Test",
    "id": 1,
    "score": 1.0,
    "active": true,
    "arr": [1, 2, 3]
  })");
  EXPECT_EQ(obj.name, "Test");
  printPass("多余嵌套数组");
}

TEST_CASE(extra_null_value) {
  auto obj = yuri::from_json<SmallObj>(R"({
    "name": "Test",
    "id": 1,
    "score": 1.0,
    "active": true,
    "nothing": null
  })");
  EXPECT_EQ(obj.name, "Test");
  printPass("多余null值");
}

// --- struct 有字段、JSON 没有（缺失字段取默认值） ---

TEST_CASE(missing_name) {
  auto obj = yuri::from_json<SmallObj>(R"({
    "id": 1,
    "score": 1.0,
    "active": true
  })");
  EXPECT_EQ(obj.name, "");
  printPass("缺失string字段→空字符串");
}

TEST_CASE(missing_id) {
  auto obj = yuri::from_json<SmallObj>(R"({
    "name": "Test",
    "score": 1.0,
    "active": true
  })");
  EXPECT_EQ(obj.id, 0);
  printPass("缺失int字段→0");
}

TEST_CASE(missing_score) {
  auto obj = yuri::from_json<SmallObj>(R"({
    "name": "Test",
    "id": 1,
    "active": true
  })");
  EXPECT_NEAR(obj.score, 0.0, 0.001);
  printPass("缺失double字段→0.0");
}

TEST_CASE(missing_active) {
  auto obj = yuri::from_json<SmallObj>(R"({
    "name": "Test",
    "id": 1,
    "score": 1.0
  })");
  EXPECT(!obj.active);
  printPass("缺失bool字段→false");
}

TEST_CASE(missing_vector_fields) {
  auto obj = yuri::from_json<MediumObj>(R"({
    "title": "Test",
    "count": 1,
    "ratio": 1.0,
    "enabled": true
  })");
  EXPECT_EQ(obj.tags.size(), 0u);
  EXPECT_EQ(obj.scores.size(), 0u);
  printPass("缺失vector字段→空数组");
}

TEST_CASE(missing_all_fields) {
  auto obj = yuri::from_json<SmallObj>(R"({})");
  EXPECT_EQ(obj.name, "");
  EXPECT_EQ(obj.id, 0);
  EXPECT_NEAR(obj.score, 0.0, 0.001);
  EXPECT(!obj.active);
  printPass("缺失全部字段→全默认值");
}

// --- 类型不匹配 ---

TEST_CASE(string_for_int) {
  auto input = R"({"name":"Test","id":"not_a_number","score":1.0,"active":true})";
  printJsonInput(input);
  EXPECT_DIAGNOSTIC(input, SmallObj);
  printDiagnostic(diag_err);
  printPass("string给int字段");
}

TEST_CASE(string_for_double) {
  auto input = R"({"name":"Test","id":1,"score":"not_a_number","active":true})";
  printJsonInput(input);
  EXPECT_DIAGNOSTIC(input, SmallObj);
  printDiagnostic(diag_err);
  printPass("string给double字段");
}

TEST_CASE(string_for_bool) {
  auto input = R"({"name":"Test","id":1,"score":1.0,"active":"yes"})";
  printJsonInput(input);
  EXPECT_DIAGNOSTIC(input, SmallObj);
  printDiagnostic(diag_err);
  printPass("string给bool字段");
}

TEST_CASE(int_for_string) {
  auto input = R"({"name":123,"id":1,"score":1.0,"active":true})";
  printJsonInput(input);
  EXPECT_DIAGNOSTIC(input, SmallObj);
  printDiagnostic(diag_err);
  printPass("int给string字段");
}

TEST_CASE(bool_for_string) {
  auto input = R"({"name":true,"id":1,"score":1.0,"active":true})";
  printJsonInput(input);
  EXPECT_DIAGNOSTIC(input, SmallObj);
  printDiagnostic(diag_err);
  printPass("bool给string字段");
}

TEST_CASE(array_for_scalar) {
  auto input = R"({"name":["a","b"],"id":1,"score":1.0,"active":true})";
  printJsonInput(input);
  EXPECT_DIAGNOSTIC(input, SmallObj);
  printDiagnostic(diag_err);
  printPass("数组给标量字段");
}

TEST_CASE(scalar_for_array) {
  auto input =
    R"({"title":"t","count":1,"ratio":1.0,"enabled":true,"tags":"not_array","scores":[]})";
  printJsonInput(input);
  EXPECT_DIAGNOSTIC(input, MediumObj);
  printDiagnostic(diag_err);
  printPass("标量给vector字段");
}

TEST_CASE(object_for_scalar) {
  auto input = R"({"name":{"nested":1},"id":1,"score":1.0,"active":true})";
  printJsonInput(input);
  EXPECT_DIAGNOSTIC(input, SmallObj);
  printDiagnostic(diag_err);
  printPass("对象给标量字段");
}

TEST_CASE(null_for_string) {
  auto input = R"({"name":null,"id":1,"score":1.0,"active":true})";
  printJsonInput(input);
  EXPECT_DIAGNOSTIC(input, SmallObj);
  printDiagnostic(diag_err);
  printPass("null给string字段");
}

// --- 同时多余和缺失 ---

TEST_CASE(extra_and_missing) {
  // 缺少 name，多余 unknown
  auto obj = yuri::from_json<SmallObj>(R"({
    "id": 42,
    "score": 3.14,
    "active": true,
    "unknown": "value"
  })");
  EXPECT_EQ(obj.name, "");
  EXPECT_EQ(obj.id, 42);
  printPass("同时多余和缺失字段");
}

// ============================================================================
// 主函数
// ============================================================================

int main() {
  printHeader("字段不匹配测试");

  // 多余字段
  test_extra_single_field();
  test_extra_multiple_fields();
  test_extra_nested_object();
  test_extra_nested_array();
  test_extra_null_value();

  // 缺失字段
  test_missing_name();
  test_missing_id();
  test_missing_score();
  test_missing_active();
  test_missing_vector_fields();
  test_missing_all_fields();

  // 类型不匹配
  test_string_for_int();
  test_string_for_double();
  test_string_for_bool();
  test_int_for_string();
  test_bool_for_string();
  test_array_for_scalar();
  test_scalar_for_array();
  test_object_for_scalar();
  test_null_for_string();

  // 组合
  test_extra_and_missing();

  printSummary();
  return g_fail > 0 ? 1 : 0;
}
