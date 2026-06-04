#include "test_common.hpp"

// ============================================================================
// JSON 结构错误测试：各种语法错误，验证库的拒绝能力 + 诊断输出
// ============================================================================

TEST_CASE(empty_string) {
  printJsonInput("");
  EXPECT_DIAGNOSTIC("", SmallObj);
  EXPECT(diag_err.context().line >= 1);
  printPass("空字符串拒绝");
}

TEST_CASE(whitespace_only) {
  printJsonInput("   \n\t  ");
  EXPECT_DIAGNOSTIC("   \n\t  ", SmallObj);
  printPass("纯空白拒绝");
}

TEST_CASE(truncated_missing_brace) {
  auto input = R"({"name":"Bob","id":42)";
  printJsonInput(input);
  EXPECT_DIAGNOSTIC(input, SmallObj);
  printDiagnostic(diag_err);
  printPass("截断-缺少右花括号");
}

TEST_CASE(truncated_missing_bracket) {
  auto input = R"({"title":"t","count":1,"ratio":1.0,"enabled":true,"tags":["a","b")";
  printJsonInput(input);
  EXPECT_DIAGNOSTIC(input, MediumObj);
  printDiagnostic(diag_err);
  printPass("截断-缺少右方括号");
}

TEST_CASE(truncated_mid_string) {
  auto input = R"({"name":"unterminated)";
  printJsonInput(input);
  EXPECT_DIAGNOSTIC(input, SmallObj);
  printDiagnostic(diag_err);
  printPass("截断-字符串未关闭");
}

TEST_CASE(truncated_mid_key) {
  auto input = R"({"name")";
  printJsonInput(input);
  EXPECT_DIAGNOSTIC(input, SmallObj);
  printDiagnostic(diag_err);
  printPass("截断-key后缺少值");
}

TEST_CASE(missing_quote_key) {
  auto input = R"({name:"Bob","id":1,"score":1.0,"active":true})";
  printJsonInput(input);
  EXPECT_DIAGNOSTIC(input, SmallObj);
  printDiagnostic(diag_err);
  printPass("缺少引号-key");
}

TEST_CASE(missing_quote_value) {
  auto input = R"({"name":Bob,"id":1,"score":1.0,"active":true})";
  printJsonInput(input);
  EXPECT_DIAGNOSTIC(input, SmallObj);
  printDiagnostic(diag_err);
  printPass("缺少引号-value");
}

TEST_CASE(missing_colon) {
  auto input = R"({"name" "Bob","id":1,"score":1.0,"active":true})";
  printJsonInput(input);
  EXPECT_DIAGNOSTIC(input, SmallObj);
  printDiagnostic(diag_err);
  printPass("缺少冒号");
}

TEST_CASE(missing_comma_between_fields) {
  auto input = R"({"name":"Bob""id":1,"score":1.0,"active":true})";
  printJsonInput(input);
  EXPECT_DIAGNOSTIC(input, SmallObj);
  printDiagnostic(diag_err);
  printPass("字段间缺少逗号");
}

TEST_CASE(missing_comma_in_array) {
  auto input = R"({"title":"t","count":1,"ratio":1.0,"enabled":true,"tags":["a""b"],"scores":[]})";
  printJsonInput(input);
  EXPECT_DIAGNOSTIC(input, MediumObj);
  printDiagnostic(diag_err);
  printPass("数组内缺少逗号");
}

TEST_CASE(double_comma) {
  auto input = R"({"name":"Bob",,"id":1,"score":1.0,"active":true})";
  printJsonInput(input);
  EXPECT_DIAGNOSTIC(input, SmallObj);
  printDiagnostic(diag_err);
  printPass("双重逗号");
}

TEST_CASE(trailing_comma_object) {
  auto input = R"({"name":"Bob","id":1,"score":1.0,"active":true,})";
  printJsonInput(input);
  EXPECT_DIAGNOSTIC(input, SmallObj);
  printDiagnostic(diag_err);
  printPass("对象尾部逗号");
}

TEST_CASE(trailing_comma_array) {
  auto input = R"({"title":"t","count":1,"ratio":1.0,"enabled":true,"tags":["a",],"scores":[]})";
  printJsonInput(input);
  EXPECT_DIAGNOSTIC(input, MediumObj);
  printDiagnostic(diag_err);
  printPass("数组尾部逗号");
}

TEST_CASE(unclosed_object) {
  auto input = R"({"name":"Bob","id":1,"score":1.0,"active":true")";
  printJsonInput(input);
  EXPECT_DIAGNOSTIC(input, SmallObj);
  printDiagnostic(diag_err);
  printPass("未关闭的对象");
}

TEST_CASE(unclosed_array) {
  auto input = R"({"title":"t","count":1,"ratio":1.0,"enabled":true,"tags":["a","b","scores":[]})";
  printJsonInput(input);
  EXPECT_DIAGNOSTIC(input, MediumObj);
  printDiagnostic(diag_err);
  printPass("未关闭的数组");
}

TEST_CASE(trailing_content) {
  auto input = R"({"name":"Bob","id":1,"score":1.0,"active":true} extra stuff)";
  printJsonInput(input);
  EXPECT_DIAGNOSTIC(input, SmallObj);
  printDiagnostic(diag_err);
  printPass("尾部多余内容");
}

TEST_CASE(garbage_input) {
  auto input = "this is not json at all";
  printJsonInput(input);
  EXPECT_DIAGNOSTIC(input, SmallObj);
  printDiagnostic(diag_err);
  printPass("完全无效输入");
}

TEST_CASE(bare_value_string) {
  auto input = R"("hello")";
  printJsonInput(input);
  EXPECT_DIAGNOSTIC(input, SmallObj);
  printDiagnostic(diag_err);
  printPass("裸字符串（非对象）");
}

TEST_CASE(bare_value_number) {
  auto input = "42";
  printJsonInput(input);
  EXPECT_DIAGNOSTIC(input, SmallObj);
  printDiagnostic(diag_err);
  printPass("裸数字（非对象）");
}

TEST_CASE(bare_value_array) {
  auto input = R"(["a","b","c"])";
  printJsonInput(input);
  EXPECT_DIAGNOSTIC(input, SmallObj);
  printDiagnostic(diag_err);
  printPass("裸数组（非对象）");
}

TEST_CASE(invalid_escape) {
  auto input = R"({"name":"bad\qescape","id":1,"score":1.0,"active":true})";
  printJsonInput(input);
  EXPECT_DIAGNOSTIC(input, SmallObj);
  printDiagnostic(diag_err);
  printPass("无效转义序列");
}

TEST_CASE(invalid_unicode_escape) {
  auto input = R"({"name":"bad\uXXXX","id":1,"score":1.0,"active":true})";
  printJsonInput(input);
  EXPECT_DIAGNOSTIC(input, SmallObj);
  printDiagnostic(diag_err);
  printPass("无效Unicode转义");
}

// ============================================================================
// 主函数
// ============================================================================

int main() {
  printHeader("JSON 结构错误测试");

  test_empty_string();
  test_whitespace_only();
  test_truncated_missing_brace();
  test_truncated_missing_bracket();
  test_truncated_mid_string();
  test_truncated_mid_key();
  test_missing_quote_key();
  test_missing_quote_value();
  test_missing_colon();
  test_missing_comma_between_fields();
  test_missing_comma_in_array();
  test_double_comma();
  test_trailing_comma_object();
  test_trailing_comma_array();
  test_unclosed_object();
  test_unclosed_array();
  test_trailing_content();
  test_garbage_input();
  test_bare_value_string();
  test_bare_value_number();
  test_bare_value_array();
  test_invalid_escape();
  test_invalid_unicode_escape();

  printSummary();
  return g_fail > 0 ? 1 : 0;
}
