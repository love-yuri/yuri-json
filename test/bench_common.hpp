#pragma once

#include <chrono>
#include <string>
#include <vector>
#include <print>

// ============================================================================
// 共享数据结构定义
// ============================================================================

/** @brief 简单结构体，包含基础类型 */
struct SmallObj {
  std::string name; // 名称
  int id;           // 标识符
  double score;     // 分数
  bool active;      // 是否激活
};

/** @brief 中等结构体，包含嵌套vector */
struct MediumObj {
  std::string title;             // 标题
  int count;                     // 计数
  double ratio;                  // 比率
  bool enabled;                  // 是否启用
  std::vector<std::string> tags; // 标签列表
  std::vector<int> scores;       // 分数列表
};

/** @brief 复杂结构体，包含大型数组 */
struct ComplexObj {
  std::string name;                 // 姓名
  int age;                          // 年龄
  double height;                    // 身高
  bool is_student;                  // 是否学生
  std::vector<std::string> hobbies; // 爱好列表
  std::vector<double> metrics;      // 指标列表
};

// ============================================================================
// JSON字符串生成函数
// ============================================================================

/** @brief 生成SmallObj的JSON字符串 */
inline std::string makeSmallJson() {
  return R"({"name":"Alice","id":42,"score":3.14,"active":true})";
}

/** @brief 生成MediumObj的JSON字符串 */
inline std::string makeMediumJson() {
  std::string json = R"({"title":"Benchmark Test","count":100,"ratio":0.95,)";
  json += R"("enabled":true,"tags":["tag0","tag1","tag2","tag3","tag4"],)";
  json += R"("scores":[10,20,30,40,50,60,70,80,90,100]})";
  return json;
}

/** @brief 生成ComplexObj的JSON字符串 */
inline std::string makeComplexJson() {
  std::string json = R"({"name":"BenchmarkUser","age":25,"height":1.75,)";
  json += R"("is_student":false,"hobbies":["reading","coding","gaming","music","sports"],)";
  json += R"("metrics":[1.1,2.2,3.3,4.4,5.5,6.6,7.7,8.8,9.9,10.0,)";
  json += R"(11.1,12.2,13.3,14.4,15.5,16.6,17.7,18.8,19.9,20.0]})";
  return json;
}

/** @brief 创建SmallObj测试对象 */
inline SmallObj makeSmallObj() {
  return { "Alice", 42, 3.14, true };
}

/** @brief 创建MediumObj测试对象 */
inline MediumObj makeMediumObj() {
  return {
    "Benchmark Test",                           // title
    100,                                        // count
    0.95,                                       // ratio
    true,                                       // enabled
    { "tag0", "tag1", "tag2", "tag3", "tag4" }, // tags
    { 10, 20, 30, 40, 50, 60, 70, 80, 90, 100 } // scores
  };
}

/** @brief 创建ComplexObj测试对象 */
inline ComplexObj makeComplexObj() {
  return {
    "BenchmarkUser",                                      // name
    25,                                                   // age
    1.75,                                                 // height
    false,                                                // is_student
    { "reading", "coding", "gaming", "music", "sports" }, // hobbies
    { 1.1,  2.2,  3.3,  4.4,  5.5,  6.6,  7.7,  8.8,  9.9,  10.0,
      11.1, 12.2, 13.3, 14.4, 15.5, 16.6, 17.7, 18.8, 19.9, 20.0 } // metrics
  };
}

// ============================================================================
// Benchmark计时工具
// ============================================================================

constexpr int kBenchIterations = 100000; // 迭代次数
constexpr int kBenchWarmup = 1000;       // 预热次数

/** @brief 计时结果 */
struct BenchResult {
  std::string name;      // 测试名称
  double serialize_us;   // 序列化耗时(微秒/操作)
  double deserialize_us; // 反序列化耗时(微秒/操作)
};

/** @brief 运行序列化benchmark并返回每次操作的微秒数 */
template <typename Func>
inline double runSerializeBench(const std::string &name, Func fn) {
  // 预热
  for (int i = 0; i < kBenchWarmup; ++i) {
    volatile auto result = fn();
  }
  // 计时
  auto t0 = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < kBenchIterations; ++i) {
    volatile auto result = fn();
  }
  auto t1 = std::chrono::high_resolution_clock::now();
  auto total_us = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
  return static_cast<double>(total_us) / kBenchIterations;
}

/** @brief 运行反序列化benchmark并返回每次操作的微秒数 */
template <typename Func>
inline double runDeserializeBench(const std::string &name, Func fn) {
  // 预热
  for (int i = 0; i < kBenchWarmup; ++i) {
    volatile auto result = fn();
  }
  // 计时
  auto t0 = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < kBenchIterations; ++i) {
    volatile auto result = fn();
  }
  auto t1 = std::chrono::high_resolution_clock::now();
  auto total_us = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
  return static_cast<double>(total_us) / kBenchIterations;
}

/** @brief 打印benchmark结果 */
inline void printResult(const std::string &lib_name, const std::vector<BenchResult> &results) {
  std::println("=== {} ===", lib_name);
  std::println("{:<25} {:>12} {:>12}", "Benchmark", "Serialize", "Deserialize");
  std::println("{:<25} {:>12} {:>12}", "---------", "---------", "-----------");
  for (const auto &r : results) {
    if (r.serialize_us < 0) {
      std::println("{:<25} {:>12} {:>9.3f} us", r.name, "N/A", r.deserialize_us);
    } else {
      std::println("{:<25} {:>9.3f} us {:>9.3f} us", r.name, r.serialize_us, r.deserialize_us);
    }
  }
  std::println("");
}
