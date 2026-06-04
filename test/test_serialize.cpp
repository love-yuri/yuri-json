#include "test_common.hpp"

// ============================================================================
// 序列化测试（to_json）
// ============================================================================

TEST_CASE(basic_types) {
  SmallObj obj{ "Alice", 42, 3.14, true };
  auto json = yuri::to_json(obj);
  EXPECT(json.find("\"name\"") != std::string::npos);
  EXPECT(json.find("\"Alice\"") != std::string::npos);
  EXPECT(json.find("42") != std::string::npos);
  EXPECT(json.find("3.14") != std::string::npos);
  EXPECT(json.find("true") != std::string::npos);
  printPass("基础类型序列化");
}

TEST_CASE(empty_string) {
  SmallObj obj{ "", 0, 0.0, false };
  auto json = yuri::to_json(obj);
  EXPECT(json.find("\"\"") != std::string::npos);
  EXPECT(json.find("false") != std::string::npos);
  printPass("空字符串序列化");
}

TEST_CASE(empty_arrays) {
  MediumObj obj{ "test", 0, 1.0, true, {}, {} };
  auto json = yuri::to_json(obj);
  EXPECT(json.find("[]") != std::string::npos);
  printPass("空数组序列化");
}

TEST_CASE(negative_numbers) {
  SmallObj obj{ "neg", -100, -3.14, false };
  auto json = yuri::to_json(obj);
  EXPECT(json.find("-100") != std::string::npos);
  EXPECT(json.find("-3.14") != std::string::npos);
  printPass("负数序列化");
}

TEST_CASE(chinese_characters) {
  SmallObj obj{ "张三", 1, 1.0, true };
  auto json = yuri::to_json(obj);
  EXPECT(json.find("张三") != std::string::npos);
  printPass("中文字符序列化");
}

TEST_CASE(escape_newline_tab) {
  SmallObj obj{ "line1\nline2\ttab", 1, 1.0, true };
  auto json = yuri::to_json(obj);
  // JSON中换行应转义为 \n（两字符），tab转义为 \t
  EXPECT(json.find(std::string{ '\\', 'n' }) != std::string::npos);
  EXPECT(json.find(std::string{ '\\', 't' }) != std::string::npos);
  printPass("换行/Tab转义序列化");
}

TEST_CASE(escape_backslash_quote) {
  SmallObj obj{ "back\\slash\"quote", 1, 1.0, true };
  auto json = yuri::to_json(obj);
  EXPECT(json.find(std::string{ '\\', '\\' }) != std::string::npos);
  EXPECT(json.find(std::string{ '\\', '"' }) != std::string::npos);
  printPass("反斜杠/引号转义序列化");
}

TEST_CASE(escape_control_chars) {
  // 包含各种控制字符
  SmallObj obj{ std::string{ '\x01', '\x02', '\x1f' }, 1, 1.0, true };
  auto json = yuri::to_json(obj);
  // 控制字符应被转义为 \uXXXX
  EXPECT(json.find("\\u00") != std::string::npos);
  printPass("控制字符转义序列化");
}

TEST_CASE(string_vector) {
  MediumObj obj{ "test", 1, 1.0, true, { "alpha", "beta", "gamma" }, {} };
  auto json = yuri::to_json(obj);
  EXPECT(json.find("\"alpha\"") != std::string::npos);
  EXPECT(json.find("\"beta\"") != std::string::npos);
  EXPECT(json.find("\"gamma\"") != std::string::npos);
  printPass("vector<string> 序列化");
}

TEST_CASE(int_vector) {
  MediumObj obj{ "test", 1, 1.0, true, {}, { 10, 20, 30 } };
  auto json = yuri::to_json(obj);
  EXPECT(json.find("10") != std::string::npos);
  EXPECT(json.find("20") != std::string::npos);
  EXPECT(json.find("30") != std::string::npos);
  printPass("vector<int> 序列化");
}

TEST_CASE(double_vector) {
  ComplexObj obj{ "test", 1, 1.0, true, {}, { 1.1, 2.2, 3.3 } };
  auto json = yuri::to_json(obj);
  EXPECT(json.find("1.1") != std::string::npos);
  EXPECT(json.find("2.2") != std::string::npos);
  EXPECT(json.find("3.3") != std::string::npos);
  printPass("vector<double> 序列化");
}

TEST_CASE(zero_values) {
  SmallObj obj{ "", 0, 0.0, false };
  auto json = yuri::to_json(obj);
  EXPECT(json.find("0") != std::string::npos);
  EXPECT(json.find("false") != std::string::npos);
  printPass("零值序列化");
}

// ============================================================================
// 主函数
// ============================================================================

int main() {
  printHeader("序列化测试 (to_json)");

  test_basic_types();
  test_empty_string();
  test_empty_arrays();
  test_negative_numbers();
  test_chinese_characters();
  test_escape_newline_tab();
  test_escape_backslash_quote();
  test_escape_control_chars();
  test_string_vector();
  test_int_vector();
  test_double_vector();
  test_zero_values();

  printSummary();
  return g_fail > 0 ? 1 : 0;
}
