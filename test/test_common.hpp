#pragma once

import yuri.json;
import std;

// ============================================================================
// 彩色终端输出
// ============================================================================

namespace color {

inline constexpr const char *kReset = "\033[0m";   // 重置
inline constexpr const char *kGreen = "\033[32m";  // 绿色
inline constexpr const char *kRed = "\033[31m";    // 红色
inline constexpr const char *kCyan = "\033[36m";   // 青色
inline constexpr const char *kYellow = "\033[33m"; // 黄色
inline constexpr const char *kDim = "\033[2m";     // 暗色
inline constexpr const char *kBold = "\033[1m";    // 粗体
inline constexpr const char *kWhite = "\033[37m";  // 白色

} // namespace color

// ============================================================================
// 全局统计
// ============================================================================

inline int g_pass = 0; // 通过计数
inline int g_fail = 0; // 失败计数

// ============================================================================
// 输出工具函数
// ============================================================================

/** @brief 打印分类标题 */
inline void printHeader(const char *category) {
  std::println("");
  std::println(
    "{}══════════════════════════════════════════════════{}", color::kCyan, color::kReset
  );
  std::println("{}  {}{}", color::kCyan, category, color::kReset);
  std::println(
    "{}══════════════════════════════════════════════════{}", color::kCyan, color::kReset
  );
}

/** @brief 打印通过的测试 */
inline void printPass(const char *name) {
  std::println("  {}✓ {}{}", color::kGreen, name, color::kReset);
  ++g_pass;
}

/** @brief 打印失败的测试 */
inline void printFail(const char *name, const char *file, int line) {
  std::println("  {}✗ {}{} ({}:{}{})", color::kRed, name, color::kDim, file, line, color::kReset);
}

/** @brief 缩进展示 JSON 输入（最多显示200字符） */
inline void printJsonInput(std::string_view json) {
  constexpr std::size_t kMaxLen = 200;
  if (json.size() <= kMaxLen) {
    std::println("    {}输入:{} {}", color::kDim, color::kReset, json);
  } else {
    std::println("    {}输入:{} {}...", color::kDim, color::kReset, json.substr(0, kMaxLen));
  }
}

/** @brief 打印诊断错误详情 */
inline void printDiagnostic(const yuri::JsonParseError &e) {
  std::println("    {}诊断:{}", color::kYellow, color::kReset);
  std::println("      {}", e.what());
  std::println("");
}

/** @brief 打印测试总结 */
inline void printSummary() {
  std::println("");
  std::println(
    "{}──────────────────────────────────────────────────{}", color::kDim, color::kReset
  );
  if (g_fail == 0) {
    std::println("  {}结果: {} 通过, {} 失败{}", color::kGreen, g_pass, g_fail, color::kReset);
  } else {
    std::println("  {}结果: {} 通过, {} 失败{}", color::kRed, g_pass, g_fail, color::kReset);
  }
  std::println(
    "{}══════════════════════════════════════════════════{}", color::kCyan, color::kReset
  );
}

// ============================================================================
// 测试框架宏
// ============================================================================

/** @brief 定义测试函数 */
#define TEST_CASE(name) static void test_##name()

/** @brief 运行单个测试（带异常捕获） */
#define RUN(fn)                                                                                    \
  do {                                                                                             \
    try {                                                                                          \
      fn();                                                                                        \
    } catch (const std::exception &e) {                                                            \
      std::println("  {}✗ {} threw: {}{}", color::kRed, #fn, e.what(), color::kReset);             \
      ++g_fail;                                                                                    \
      break;                                                                                       \
    } catch (...) {                                                                                \
      std::println("  {}✗ {} threw unknown{}", color::kRed, #fn, color::kReset);                   \
      ++g_fail;                                                                                    \
      break;                                                                                       \
    }                                                                                              \
    ++g_pass;                                                                                      \
  } while (0)

/** @brief 断言为真（失败继续） */
#define EXPECT(expr)                                                                               \
  do {                                                                                             \
    if (!(expr)) {                                                                                 \
      printFail(#expr, __FILE__, __LINE__);                                                        \
      ++g_fail;                                                                                    \
      return;                                                                                      \
    }                                                                                              \
  } while (0)

/** @brief 断言相等 */
#define EXPECT_EQ(a, b)                                                                            \
  do {                                                                                             \
    if ((a) != (b)) {                                                                              \
      printFail(#a " == " #b, __FILE__, __LINE__);                                                 \
      ++g_fail;                                                                                    \
      return;                                                                                      \
    }                                                                                              \
  } while (0)

/** @brief 断言浮点近似 */
#define EXPECT_NEAR(a, b, eps)                                                                     \
  do {                                                                                             \
    if (std::abs((a) - (b)) > (eps)) {                                                             \
      printFail(#a " ≈ " #b, __FILE__, __LINE__);                                                  \
      ++g_fail;                                                                                    \
      return;                                                                                      \
    }                                                                                              \
  } while (0)

/** @brief 断言应抛出指定类型异常 */
#define EXPECT_THROW(expr, ExType)                                                                 \
  do {                                                                                             \
    bool caught = false;                                                                           \
    try {                                                                                          \
      expr;                                                                                        \
    } catch (const ExType &) {                                                                     \
      caught = true;                                                                               \
    } catch (const std::exception &e) {                                                            \
      std::println("    {}期望 {}，实际: {}{}", color::kRed, #ExType, e.what(), color::kReset);    \
      ++g_fail;                                                                                    \
      return;                                                                                      \
    }                                                                                              \
    if (!caught) {                                                                                 \
      printFail("期望抛出 " #ExType, __FILE__, __LINE__);                                          \
      ++g_fail;                                                                                    \
      return;                                                                                      \
    }                                                                                              \
  } while (0)

/** @brief 断言不应抛出异常 */
#define EXPECT_NO_THROW(expr)                                                                      \
  do {                                                                                             \
    try {                                                                                          \
      expr;                                                                                        \
    } catch (const std::exception &e) {                                                            \
      std::println("    {}不应抛异常，实际: {}{}", color::kRed, e.what(), color::kReset);          \
      ++g_fail;                                                                                    \
      return;                                                                                      \
    }                                                                                              \
  } while (0)

/** @brief 断言应抛出 JsonParseError 并获取诊断信息 */
#define EXPECT_DIAGNOSTIC(json_str, T)                                                             \
  yuri::JsonParseError diag_err("", {});                                                           \
  bool diag_caught = false;                                                                        \
  try {                                                                                            \
    yuri::from_json<T>(json_str, yuri::kWithDiagnostics);                                          \
  } catch (const yuri::JsonParseError &e) {                                                        \
    diag_err = e;                                                                                  \
    diag_caught = true;                                                                            \
  }                                                                                                \
  if (!diag_caught) {                                                                              \
    printFail("期望 JsonParseError", __FILE__, __LINE__);                                          \
    ++g_fail;                                                                                      \
    return;                                                                                        \
  }

// ============================================================================
// 测试数据结构
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

/** @brief 复杂结构体，包含double数组 */
struct ComplexObj {
  std::string name;                 // 姓名
  int age;                          // 年龄
  double height;                    // 身高
  bool is_student;                  // 是否学生
  std::vector<std::string> hobbies; // 爱好列表
  std::vector<double> metrics;      // 指标列表
};
