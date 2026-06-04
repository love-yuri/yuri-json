#include "test_common.hpp"

// ============================================================================
// Round-trip 测试：to_json → from_json 一致性
// ============================================================================

TEST_CASE(small_object) {
  SmallObj orig{ "RoundTrip", 123, 9.81, true };
  auto json = yuri::to_json(orig);
  auto obj = yuri::from_json<SmallObj>(json);
  EXPECT_EQ(obj.name, orig.name);
  EXPECT_EQ(obj.id, orig.id);
  EXPECT_NEAR(obj.score, orig.score, 0.001);
  EXPECT_EQ(obj.active, orig.active);
  printPass("SmallObj round-trip");
}

TEST_CASE(medium_object) {
  MediumObj orig{ "Medium", 50, 0.75, true, { "t1", "t2", "t3" }, { 100, 200, 300 } };
  auto json = yuri::to_json(orig);
  auto obj = yuri::from_json<MediumObj>(json);
  EXPECT_EQ(obj.title, orig.title);
  EXPECT_EQ(obj.count, orig.count);
  EXPECT_NEAR(obj.ratio, orig.ratio, 0.001);
  EXPECT_EQ(obj.enabled, orig.enabled);
  EXPECT_EQ(obj.tags.size(), orig.tags.size());
  EXPECT_EQ(obj.tags[0], orig.tags[0]);
  EXPECT_EQ(obj.tags[2], orig.tags[2]);
  EXPECT_EQ(obj.scores.size(), orig.scores.size());
  EXPECT_EQ(obj.scores[1], orig.scores[1]);
  printPass("MediumObj round-trip");
}

TEST_CASE(complex_object) {
  ComplexObj orig{
    "Complex", 25, 1.75, false, { "reading", "coding", "gaming" }, { 1.1, 2.2, 3.3, 4.4, 5.5 }
  };
  auto json = yuri::to_json(orig);
  auto obj = yuri::from_json<ComplexObj>(json);
  EXPECT_EQ(obj.name, orig.name);
  EXPECT_EQ(obj.age, orig.age);
  EXPECT_NEAR(obj.height, orig.height, 0.001);
  EXPECT_EQ(obj.is_student, orig.is_student);
  EXPECT_EQ(obj.hobbies.size(), orig.hobbies.size());
  EXPECT_EQ(obj.hobbies[0], orig.hobbies[0]);
  EXPECT_EQ(obj.hobbies[2], orig.hobbies[2]);
  EXPECT_EQ(obj.metrics.size(), orig.metrics.size());
  EXPECT_NEAR(obj.metrics[3], orig.metrics[3], 0.001);
  printPass("ComplexObj round-trip");
}

TEST_CASE(large_object) {
  MediumObj orig;
  orig.title = "Large Round Trip";
  orig.count = 999;
  orig.ratio = 0.123;
  orig.enabled = true;
  for (int i = 0; i < 100; ++i) {
    orig.tags.push_back("tag_" + std::to_string(i));
  }
  for (int i = 0; i < 500; ++i) {
    orig.scores.push_back(i * 7 + 3);
  }
  auto json = yuri::to_json(orig);
  auto obj = yuri::from_json<MediumObj>(json);
  EXPECT_EQ(obj.title, orig.title);
  EXPECT_EQ(obj.tags.size(), 100u);
  EXPECT_EQ(obj.tags[99], "tag_99");
  EXPECT_EQ(obj.scores.size(), 500u);
  EXPECT_EQ(obj.scores[0], 3);
  EXPECT_EQ(obj.scores[499], 499 * 7 + 3);
  printPass("LargeObj round-trip（~15KB）");
}

TEST_CASE(escape_characters) {
  SmallObj orig{ "line1\nline2\ttab\\backslash\"quote\x01\x02", 1, 1.0, true };
  auto json = yuri::to_json(orig);
  auto obj = yuri::from_json<SmallObj>(json);
  EXPECT_EQ(obj.name, orig.name);
  printPass("转义字符 round-trip");
}

TEST_CASE(chinese_unicode) {
  SmallObj orig{ "张三李四王五", 1, 1.0, true };
  auto json = yuri::to_json(orig);
  auto obj = yuri::from_json<SmallObj>(json);
  EXPECT_EQ(obj.name, orig.name);
  printPass("中文/Unicode round-trip");
}

TEST_CASE(all_empty) {
  MediumObj orig{ "", 0, 0.0, false, {}, {} };
  auto json = yuri::to_json(orig);
  auto obj = yuri::from_json<MediumObj>(json);
  EXPECT_EQ(obj.title, "");
  EXPECT_EQ(obj.count, 0);
  EXPECT_EQ(obj.tags.size(), 0u);
  EXPECT_EQ(obj.scores.size(), 0u);
  printPass("全空值 round-trip");
}

// ============================================================================
// 主函数
// ============================================================================

int main() {
  printHeader("Round-trip 测试 (to_json → from_json)");

  test_small_object();
  test_medium_object();
  test_complex_object();
  test_large_object();
  test_escape_characters();
  test_chinese_unicode();
  test_all_empty();

  printSummary();
  return g_fail > 0 ? 1 : 0;
}
