#include "test_common.hpp"

// ============================================================================
// 诊断输出能力验证：验证 JsonParseError 的行号、列号、snippet 等
// ============================================================================

TEST_CASE(line_number_single_line) {
  auto input = R"({"name":"Bob","id":"wrong","score":1.0,"active":true})";
  EXPECT_DIAGNOSTIC(input, SmallObj);
  // 单行JSON，错误应在第1行
  EXPECT_EQ(diag_err.context().line, 1u);
  EXPECT(diag_err.context().column > 0);
  printPass("单行JSON-行号正确");
}

TEST_CASE(line_number_multi_line) {
  auto input = R"({
    "name": "Bob",
    "id": "wrong",
    "score": 1.0,
    "active": true
  })";
  EXPECT_DIAGNOSTIC(input, SmallObj);
  // "id" 在第3行
  EXPECT(diag_err.context().line >= 2u);
  EXPECT(diag_err.context().line <= 4u);
  printPass("多行JSON-行号范围正确");
}

TEST_CASE(column_accuracy) {
  auto input = R"({"name":"Bob","id":"wrong"})";
  EXPECT_DIAGNOSTIC(input, SmallObj);
  // "wrong" 在 id 字段的值位置
  EXPECT(diag_err.context().column > 0);
  printPass("列号准确性");
}

TEST_CASE(snippet_not_empty) {
  auto input = R"({"name":"Bob","id":"wrong","score":1.0,"active":true})";
  EXPECT_DIAGNOSTIC(input, SmallObj);
  EXPECT(!diag_err.context().snippet.empty());
  printPass("snippet非空");
}

TEST_CASE(snippet_contains_context) {
  auto input = R"({"name":"Bob","id":"wrong","score":1.0,"active":true})";
  EXPECT_DIAGNOSTIC(input, SmallObj);
  // snippet 应包含错误附近的内容
  auto &snippet = diag_err.context().snippet;
  EXPECT(snippet.find("id") != std::string::npos);
  printPass("snippet包含错误附近内容");
}

TEST_CASE(expected_not_empty) {
  auto input = R"({"name":"Bob","id":"wrong"})";
  EXPECT_DIAGNOSTIC(input, SmallObj);
  EXPECT(!diag_err.context().expected.empty());
  printPass("expected信息非空");
}

TEST_CASE(found_not_empty) {
  auto input = R"({"name":"Bob","id":"wrong"})";
  EXPECT_DIAGNOSTIC(input, SmallObj);
  EXPECT(!diag_err.context().found.empty());
  printPass("found信息非空");
}

TEST_CASE(caret_column_valid) {
  auto input = R"({"name":"Bob","id":"wrong"})";
  EXPECT_DIAGNOSTIC(input, SmallObj);
  EXPECT(diag_err.context().caret_column > 0);
  printPass("caret_column有效");
}

TEST_CASE(empty_input_diagnostics) {
  EXPECT_DIAGNOSTIC("", SmallObj);
  EXPECT(diag_err.context().line >= 1);
  EXPECT(!diag_err.context().expected.empty());
  printPass("空输入诊断");
}

TEST_CASE(truncated_diagnostics) {
  auto input = R"({"name":"abc")";
  EXPECT_DIAGNOSTIC(input, SmallObj);
  EXPECT(!diag_err.context().expected.empty());
  EXPECT(!diag_err.context().found.empty());
  printPass("截断输入诊断");
}

TEST_CASE(missing_colon_diagnostics) {
  auto input = R"({"name" "Bob"})";
  EXPECT_DIAGNOSTIC(input, SmallObj);
  EXPECT(diag_err.context().line >= 1);
  printPass("缺少冒号诊断");
}

TEST_CASE(missing_comma_diagnostics) {
  auto input = R"({"name":"Bob""id":1})";
  EXPECT_DIAGNOSTIC(input, SmallObj);
  EXPECT(diag_err.context().line >= 1);
  printPass("缺少逗号诊断");
}

TEST_CASE(type_mismatch_diagnostics) {
  auto input = R"({
    "name": "Bob",
    "id": "not_a_number",
    "score": 1.0,
    "active": true
  })";
  EXPECT_DIAGNOSTIC(input, SmallObj);
  EXPECT(diag_err.context().line >= 2u);
  EXPECT(diag_err.context().line <= 4u);
  // 应包含字段信息
  auto &expected = diag_err.context().expected;
  EXPECT(expected.find("id") != std::string::npos || expected.find("integer") != std::string::npos);
  printPass("类型不匹配诊断（多行）");
}

TEST_CASE(diagnostic_exception_message) {
  auto input = R"({"name":"Bob","id":"wrong"})";
  try {
    yuri::from_json<SmallObj>(input, yuri::kWithDiagnostics);
    EXPECT(false);
  } catch (const yuri::JsonParseError &e) {
    // 异常 message 应包含 ANSI 颜色码
    std::string msg = e.what();
    EXPECT(msg.find("\033[") != std::string::npos);
    // 应包含 "JSON parse error"
    EXPECT(msg.find("JSON parse error") != std::string::npos);
  }
  printPass("异常消息包含颜色码和标题");
}

TEST_CASE(offset_valid) {
  std::string_view input = R"({"name":"Bob","id":"wrong","score":1.0})";
  EXPECT_DIAGNOSTIC(input, SmallObj);
  EXPECT(diag_err.context().offset > 0);
  EXPECT(diag_err.context().offset < input.size());
  printPass("offset在有效范围内");
}

// ============================================================================
// 主函数
// ============================================================================

int main() {
  printHeader("诊断输出能力验证");

  test_line_number_single_line();
  test_line_number_multi_line();
  test_column_accuracy();
  test_snippet_not_empty();
  test_snippet_contains_context();
  test_expected_not_empty();
  test_found_not_empty();
  test_caret_column_valid();
  test_empty_input_diagnostics();
  test_truncated_diagnostics();
  test_missing_colon_diagnostics();
  test_missing_comma_diagnostics();
  test_type_mismatch_diagnostics();
  test_diagnostic_exception_message();
  test_offset_valid();

  printSummary();
  return g_fail > 0 ? 1 : 0;
}
