#include "test_common.hpp"

// ============================================================================
// 边界情况测试：各种极端输入
// ============================================================================

// --- 空值/零值 ---

TEST_CASE(empty_string_value) {
  SmallObj orig{ "", 0, 0.0, false };
  auto json = yuri::to_json(orig);
  auto obj = yuri::from_json<SmallObj>(json);
  EXPECT_EQ(obj.name, "");
  printPass("空字符串值");
}

TEST_CASE(empty_arrays_roundtrip) {
  MediumObj orig{ "t", 0, 0.0, false, {}, {} };
  auto json = yuri::to_json(orig);
  auto obj = yuri::from_json<MediumObj>(json);
  EXPECT_EQ(obj.tags.size(), 0u);
  EXPECT_EQ(obj.scores.size(), 0u);
  printPass("空数组round-trip");
}

// --- 数值边界 ---

TEST_CASE(int_max) {
  SmallObj orig{ "max", 2147483647, 1.0, true };
  auto json = yuri::to_json(orig);
  auto obj = yuri::from_json<SmallObj>(json);
  EXPECT_EQ(obj.id, 2147483647);
  printPass("INT_MAX");
}

TEST_CASE(int_min) {
  SmallObj orig{ "min", -2147483648, 1.0, true };
  auto json = yuri::to_json(orig);
  auto obj = yuri::from_json<SmallObj>(json);
  EXPECT_EQ(obj.id, -2147483648);
  printPass("INT_MIN");
}

TEST_CASE(int_zero) {
  auto obj = yuri::from_json<SmallObj>(R"({"name":"z","id":0,"score":0,"active":false})");
  EXPECT_EQ(obj.id, 0);
  printPass("int零值");
}

TEST_CASE(double_small) {
  SmallObj orig{ "small", 0, 0.0001, true };
  auto json = yuri::to_json(orig);
  auto obj = yuri::from_json<SmallObj>(json);
  EXPECT_NEAR(obj.score, 0.0001, 0.00001);
  printPass("极小浮点数");
}

TEST_CASE(double_negative) {
  auto obj = yuri::from_json<SmallObj>(R"({"name":"n","id":0,"score":-999.999,"active":false})");
  EXPECT_NEAR(obj.score, -999.999, 0.001);
  printPass("负浮点数");
}

// --- 字符串边界 ---

TEST_CASE(long_string) {
  std::string long_str(10000, 'x');
  SmallObj orig{ long_str, 1, 1.0, true };
  auto json = yuri::to_json(orig);
  auto obj = yuri::from_json<SmallObj>(json);
  EXPECT_EQ(obj.name.size(), 10000u);
  EXPECT_EQ(obj.name, long_str);
  printPass("超长字符串（10000字符）");
}

TEST_CASE(all_escape_chars) {
  // 包含所有可转义字符
  std::string s = "\"\\/\b\f\n\r\t";
  SmallObj orig{ s, 1, 1.0, true };
  auto json = yuri::to_json(orig);
  auto obj = yuri::from_json<SmallObj>(json);
  EXPECT_EQ(obj.name, s);
  printPass("所有转义字符round-trip");
}

TEST_CASE(unicode_escape_basic) {
  auto input = R"({"name":"Hello","id":1,"score":1.0,"active":true})";
  auto obj = yuri::from_json<SmallObj>(input);
  EXPECT_EQ(obj.name, "Hello");
  printPass("Unicode转义 \\u00XX");
}

TEST_CASE(unicode_escape_chinese) {
  // 张 = 张, 三 = 三
  auto input = R"({"name":"张三","id":1,"score":1.0,"active":true})";
  auto obj = yuri::from_json<SmallObj>(input);
  EXPECT_EQ(obj.name, "张三");
  printPass("Unicode转义中文");
}

// --- 数组边界 ---

TEST_CASE(single_element_array) {
  auto obj = yuri::from_json<MediumObj>(R"({
    "title":"t","count":1,"ratio":1.0,"enabled":true,
    "tags":["only"],"scores":[42]
  })");
  EXPECT_EQ(obj.tags.size(), 1u);
  EXPECT_EQ(obj.tags[0], "only");
  EXPECT_EQ(obj.scores.size(), 1u);
  EXPECT_EQ(obj.scores[0], 42);
  printPass("单元素数组");
}

TEST_CASE(large_int_array) {
  std::string json = R"({"title":"t","count":1,"ratio":1.0,"enabled":true,"tags":[],"scores":[)";
  for (int i = 0; i < 1000; ++i) {
    if (i > 0) json += ',';
    json += std::to_string(i);
  }
  json += "]}";
  auto obj = yuri::from_json<MediumObj>(json);
  EXPECT_EQ(obj.scores.size(), 1000u);
  EXPECT_EQ(obj.scores[0], 0);
  EXPECT_EQ(obj.scores[999], 999);
  printPass("大整数数组（1000元素）");
}

TEST_CASE(large_double_array) {
  std::string json =
    R"({"name":"t","age":1,"height":1.0,"is_student":false,"hobbies":[],"metrics":[)";
  for (int i = 0; i < 1000; ++i) {
    if (i > 0) json += ',';
    json += std::to_string(i * 0.1);
  }
  json += "]}";
  auto obj = yuri::from_json<ComplexObj>(json);
  EXPECT_EQ(obj.metrics.size(), 1000u);
  printPass("大浮点数组（1000元素）");
}

// --- 空白格式 ---

TEST_CASE(minified_json) {
  auto obj = yuri::from_json<SmallObj>(R"({"name":"Min","id":1,"score":1.0,"active":true})");
  EXPECT_EQ(obj.name, "Min");
  printPass("紧凑JSON（无空白）");
}

TEST_CASE(pretty_json) {
  auto obj = yuri::from_json<SmallObj>(R"(
    {
      "name" : "Pretty" ,
      "id"   : 42 ,
      "score": 3.14 ,
      "active": true
    }
  )");
  EXPECT_EQ(obj.name, "Pretty");
  EXPECT_EQ(obj.id, 42);
  printPass("格式化JSON（大量空白）");
}

// --- bool 边界 ---

TEST_CASE(bool_true) {
  auto obj = yuri::from_json<SmallObj>(R"({"name":"t","id":0,"score":0,"active":true})");
  EXPECT(obj.active);
  printPass("bool true");
}

TEST_CASE(bool_false) {
  auto obj = yuri::from_json<SmallObj>(R"({"name":"f","id":0,"score":0,"active":false})");
  EXPECT(!obj.active);
  printPass("bool false");
}

// ============================================================================
// 主函数
// ============================================================================

int main() {
  printHeader("边界情况测试");

  // 空值
  test_empty_string_value();
  test_empty_arrays_roundtrip();

  // 数值边界
  test_int_max();
  test_int_min();
  test_int_zero();
  test_double_small();
  test_double_negative();

  // 字符串边界
  test_long_string();
  test_all_escape_chars();
  test_unicode_escape_basic();
  test_unicode_escape_chinese();

  // 数组边界
  test_single_element_array();
  test_large_int_array();
  test_large_double_array();

  // 空白格式
  test_minified_json();
  test_pretty_json();

  // bool
  test_bool_true();
  test_bool_false();

  printSummary();
  return g_fail > 0 ? 1 : 0;
}
