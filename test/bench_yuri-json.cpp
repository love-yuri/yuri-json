import yuri.json;
import std;

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

/** @brief 复杂结构体，包含大型数组 */
struct ComplexObj {
  std::string name;                 // 姓名
  int age;                          // 年龄
  double height;                    // 身高
  bool is_student;                  // 是否学生
  std::vector<std::string> hobbies; // 爱好列表
  std::vector<double> metrics;      // 指标列表
};

/** @brief 大型结构体，KB级JSON */
struct LargeObj {
  std::string title;                      // 标题
  int version;                            // 版本
  bool debug;                             // 调试模式
  std::vector<std::string> tags;          // 标签
  std::vector<int> indices;               // 索引
  std::vector<double> values;             // 值
};

// ============================================================================
// JSON字符串生成函数
// ============================================================================

/** @brief 生成SmallObj的JSON字符串 */
std::string makeSmallJson() {
  return R"({"name":"Alice","id":42,"score":3.14,"active":true})";
}

/** @brief 生成MediumObj的JSON字符串 */
std::string makeMediumJson() {
  std::string json = R"({"title":"Benchmark Test","count":100,"ratio":0.95,)";
  json += R"("enabled":true,"tags":["tag0","tag1","tag2","tag3","tag4"],)";
  json += R"("scores":[10,20,30,40,50,60,70,80,90,100]})";
  return json;
}

/** @brief 生成ComplexObj的JSON字符串 */
std::string makeComplexJson() {
  std::string json = R"({"name":"BenchmarkUser","age":25,"height":1.75,)";
  json += R"("is_student":false,"hobbies":["reading","coding","gaming","music","sports"],)";
  json += R"("metrics":[1.1,2.2,3.3,4.4,5.5,6.6,7.7,8.8,9.9,10.0,)";
  json += R"(11.1,12.2,13.3,14.4,15.5,16.6,17.7,18.8,19.9,20.0]})";
  return json;
}

/** @brief 创建SmallObj测试对象 */
SmallObj makeSmallObj() {
  return { "Alice", 42, 3.14, true };
}

/** @brief 创建MediumObj测试对象 */
MediumObj makeMediumObj() {
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
ComplexObj makeComplexObj() {
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

/** @brief 生成LargeObj的JSON字符串（约15KB） */
std::string makeLargeJson() {
  std::string json;
  json.reserve(16384);
  json += R"({"title":"Large Benchmark Dataset","version":42,"debug":false,)";
  json += R"("tags":[)";
  for (int i = 0; i < 100; ++i) {
    if (i > 0) json += ',';
    json += '"';
    json += "tag_" + std::to_string(i);
    json += '"';
  }
  json += "],";
  json += R"("indices":[)";
  for (int i = 0; i < 500; ++i) {
    if (i > 0) json += ',';
    json += std::to_string(i * 7 + 3);
  }
  json += "],";
  json += R"("values":[)";
  for (int i = 0; i < 500; ++i) {
    if (i > 0) json += ',';
    json += std::to_string(i * 0.123 + 1.0);
  }
  json += "]}";
  return json;
}

/** @brief 创建LargeObj测试对象 */
LargeObj makeLargeObj() {
  LargeObj obj;
  obj.title = "Large Benchmark Dataset";
  obj.version = 42;
  obj.debug = false;
  obj.tags.reserve(100);
  for (int i = 0; i < 100; ++i)
    obj.tags.push_back("tag_" + std::to_string(i));
  obj.indices.reserve(500);
  for (int i = 0; i < 500; ++i)
    obj.indices.push_back(i * 7 + 3);
  obj.values.reserve(500);
  for (int i = 0; i < 500; ++i)
    obj.values.push_back(i * 0.123 + 1.0);
  return obj;
}

// ============================================================================
// Benchmark计时工具
// ============================================================================

constexpr int kBenchIterations = 100000; // 迭代次数
constexpr int kBenchWarmup = 1000;       // 预热次数

/** @brief 运行序列化benchmark并返回每次操作的微秒数 */
template <typename Func>
double runSerializeBench(Func fn) {
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
double runDeserializeBench(Func fn) {
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

// ============================================================================
// Benchmark主函数
// ============================================================================

/** @brief json反射库benchmark主函数 */
int main() {
  std::println("=== C++26 yuri-json JSON Benchmark ===\n");

  // SmallObj
  {
    auto obj = makeSmallObj();
    auto json_str = makeSmallJson();
    auto ser = runSerializeBench([&] { return yuri::to_json(obj); });
    auto deser = runDeserializeBench([&] { return yuri::from_json<SmallObj>(json_str); });
    std::println("{:<25} {:>9.3f} us {:>9.3f} us", "SmallObj", ser, deser);
  }

  // MediumObj
  {
    auto obj = makeMediumObj();
    auto json_str = makeMediumJson();
    auto ser = runSerializeBench([&] { return yuri::to_json(obj); });
    auto deser = runDeserializeBench([&] { return yuri::from_json<MediumObj>(json_str); });
    std::println("{:<25} {:>9.3f} us {:>9.3f} us", "MediumObj", ser, deser);
  }

  // ComplexObj
  {
    auto obj = makeComplexObj();
    auto json_str = makeComplexJson();
    auto ser = runSerializeBench([&] { return yuri::to_json(obj); });
    auto deser = runDeserializeBench([&] { return yuri::from_json<ComplexObj>(json_str); });
    std::println("{:<25} {:>9.3f} us {:>9.3f} us", "ComplexObj", ser, deser);
  }

  // LargeObj（约15KB JSON）
  {
    auto obj = makeLargeObj();
    auto json_str = makeLargeJson();
    auto ser = runSerializeBench([&] { return yuri::to_json(obj); });
    auto deser = runDeserializeBench([&] { return yuri::from_json<LargeObj>(json_str); });
    std::println("{:<25} {:>9.3f} us {:>9.3f} us", "LargeObj(~15KB)", ser, deser);
  }

  return 0;
}
