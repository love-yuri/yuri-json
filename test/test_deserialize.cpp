#include "test_common.hpp"

// ============================================================================
// 正确 JSON 反序列化测试（from_json）
// ============================================================================

TEST_CASE(basic_types) {
  auto obj = yuri::from_json<SmallObj>(R"({
    "name": "Bob",
    "id": 99,
    "score": 2.71,
    "active": false
  })");
  EXPECT_EQ(obj.name, "Bob");
  EXPECT_EQ(obj.id, 99);
  EXPECT_NEAR(obj.score, 2.71, 0.001);
  EXPECT(!obj.active);
  printPass("基础类型反序列化");
}

TEST_CASE(medium_object) {
  auto obj = yuri::from_json<MediumObj>(R"({
    "title": "hello",
    "count": 5,
    "ratio": 0.5,
    "enabled": true,
    "tags": ["x", "y"],
    "scores": [1, 2, 3]
  })");
  EXPECT_EQ(obj.title, "hello");
  EXPECT_EQ(obj.count, 5);
  EXPECT_NEAR(obj.ratio, 0.5, 0.001);
  EXPECT(obj.enabled);
  EXPECT_EQ(obj.tags.size(), 2u);
  EXPECT_EQ(obj.tags[0], "x");
  EXPECT_EQ(obj.tags[1], "y");
  EXPECT_EQ(obj.scores.size(), 3u);
  EXPECT_EQ(obj.scores[0], 1);
  EXPECT_EQ(obj.scores[2], 3);
  printPass("中等结构体反序列化");
}

TEST_CASE(empty_object) {
  auto obj = yuri::from_json<SmallObj>(R"({})");
  EXPECT_EQ(obj.name, "");
  EXPECT_EQ(obj.id, 0);
  EXPECT_NEAR(obj.score, 0.0, 0.001);
  EXPECT(!obj.active);
  printPass("空对象反序列化（默认值）");
}

TEST_CASE(empty_arrays) {
  auto obj = yuri::from_json<MediumObj>(R"({
    "title": "t",
    "count": 0,
    "ratio": 1.0,
    "enabled": false,
    "tags": [],
    "scores": []
  })");
  EXPECT_EQ(obj.tags.size(), 0u);
  EXPECT_EQ(obj.scores.size(), 0u);
  printPass("空数组反序列化");
}

TEST_CASE(shuffled_field_order) {
  // 字段顺序与struct声明不同，走hash回退路径
  auto obj = yuri::from_json<SmallObj>(R"({
    "active": true,
    "score": 1.5,
    "id": 7,
    "name": "Shuffled"
  })");
  EXPECT_EQ(obj.name, "Shuffled");
  EXPECT_EQ(obj.id, 7);
  EXPECT_NEAR(obj.score, 1.5, 0.001);
  EXPECT(obj.active);
  printPass("字段乱序反序列化（hash回退）");
}

TEST_CASE(extra_fields_ignored) {
  // JSON 中有多余字段，应被忽略
  auto obj = yuri::from_json<SmallObj>(R"({
    "name": "Extra",
    "id": 1,
    "score": 1.0,
    "active": true,
    "unknown": 42,
    "extra_field": "hello"
  })");
  EXPECT_EQ(obj.name, "Extra");
  EXPECT_EQ(obj.id, 1);
  printPass("多余字段忽略");
}

TEST_CASE(whitespace_formats) {
  // 各种空白格式：换行、tab、多空格
  auto obj = yuri::from_json<SmallObj>(
    "{\n\t \"name\" : \"Spaced\" ,\n\t \"id\" : 10 ,\n"
    "\t \"score\" : 9.9 ,\n\t \"active\" : false\n}"
  );
  EXPECT_EQ(obj.name, "Spaced");
  EXPECT_EQ(obj.id, 10);
  EXPECT_NEAR(obj.score, 9.9, 0.001);
  EXPECT(!obj.active);
  printPass("各种空白格式");
}

TEST_CASE(negative_values) {
  auto obj = yuri::from_json<SmallObj>(R"({
    "name": "neg",
    "id": -42,
    "score": -1.5,
    "active": false
  })");
  EXPECT_EQ(obj.id, -42);
  EXPECT_NEAR(obj.score, -1.5, 0.001);
  printPass("负数反序列化");
}

TEST_CASE(large_arrays) {
  // 构造一个大数组 JSON
  std::string json = R"({"title":"big","count":1,"ratio":1.0,"enabled":true,"tags":[)";
  for (int i = 0; i < 50; ++i) {
    if (i > 0) json += ',';
    json += '"';
    json += "tag_" + std::to_string(i);
    json += '"';
  }
  json += R"(],"scores":[)";
  for (int i = 0; i < 100; ++i) {
    if (i > 0) json += ',';
    json += std::to_string(i);
  }
  json += "]}";

  auto obj = yuri::from_json<MediumObj>(json);
  EXPECT_EQ(obj.tags.size(), 50u);
  EXPECT_EQ(obj.tags[49], "tag_49");
  EXPECT_EQ(obj.scores.size(), 100u);
  EXPECT_EQ(obj.scores[99], 99);
  printPass("大数组反序列化");
}

TEST_CASE(escape_roundtrip) {
  // 包含转义字符的字符串，先序列化再反序列化
  SmallObj orig{ "line1\nline2\ttab\\backslash\"quote", 1, 1.0, true };
  auto json = yuri::to_json(orig);
  auto obj = yuri::from_json<SmallObj>(json);
  EXPECT_EQ(obj.name, orig.name);
  printPass("转义字符反序列化");
}

TEST_CASE(chinese_roundtrip) {
  SmallObj orig{ "张三李四", 1, 1.0, true };
  auto json = yuri::to_json(orig);
  auto obj = yuri::from_json<SmallObj>(json);
  EXPECT_EQ(obj.name, "张三李四");
  printPass("中文字符反序列化");
}

TEST_CASE(complex_object) {
  auto obj = yuri::from_json<ComplexObj>(R"({
    "name": "User",
    "age": 30,
    "height": 1.75,
    "is_student": false,
    "hobbies": ["reading", "coding"],
    "metrics": [1.1, 2.2, 3.3]
  })");
  EXPECT_EQ(obj.name, "User");
  EXPECT_EQ(obj.age, 30);
  EXPECT_NEAR(obj.height, 1.75, 0.001);
  EXPECT(!obj.is_student);
  EXPECT_EQ(obj.hobbies.size(), 2u);
  EXPECT_EQ(obj.hobbies[1], "coding");
  EXPECT_EQ(obj.metrics.size(), 3u);
  EXPECT_NEAR(obj.metrics[2], 3.3, 0.001);
  printPass("复杂结构体反序列化");
}

// ============================================================================
// 主函数
// ============================================================================

int main() {
  printHeader("反序列化测试 (from_json)");

  test_basic_types();
  test_medium_object();
  test_empty_object();
  test_empty_arrays();
  test_shuffled_field_order();
  test_extra_fields_ignored();
  test_whitespace_formats();
  test_negative_values();
  test_large_arrays();
  test_escape_roundtrip();
  test_chinese_roundtrip();
  test_complex_object();

  printSummary();
  return g_fail > 0 ? 1 : 0;
}
