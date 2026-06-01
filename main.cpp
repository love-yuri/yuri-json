import std;
import yuri.json;

struct Address {
  std::string city;   // 城市
  std::string street; // 街道
  int zip_code;       // 邮编
};

struct Person {
  std::string name;                 // 姓名
  int age;                          // 年龄
  double height;                    // 身高
  bool is_student;                  // 是否学生
  std::vector<std::string> hobbies; // 爱好列表
};

/** @brief 将字符串vector连接为逗号分隔的字符串 */
std::string joinStrings(const std::vector<std::string> &vec) {
  std::string result;
  for (std::size_t i = 0; i < vec.size(); ++i) {
    if (i > 0) {
      result += ", ";
    }
    result += vec[i];
  }
  return result;
}

int main() {
  std::println("=== C++26 Reflect JSON (SIMD) ===\n");

  // 测试1：Person序列化
  Person p{ "Alice", 30, 1.65, false, { "reading", "coding", "gaming" } };
  std::string json_str = yuri::to_json(p);
  std::println("serialize: {}", json_str);

  // 测试2：Person反序列化
  Person p2 = yuri::from_json<Person>(json_str);
  std::println(
    "deserialize: name={}, age={}, height={}, hobbies=[{}]",
    p2.name,
    p2.age,
    p2.height,
    joinStrings(p2.hobbies)
  );

  // 测试3：外部JSON
  std::string_view ext =
    R"({"name":"Bob","age":25,"height":1.8,"is_student":true,"hobbies":["music","sports"]})";
  Person p3 = yuri::from_json<Person>(ext);
  std::println("external: name={}, age={}, hobbies=[{}]", p3.name, p3.age, joinStrings(p3.hobbies));

  // 测试4：中文
  Person p4{ "张三", 28, 1.75, false, { "编程", "游戏" } };
  std::string j4 = yuri::to_json(p4);
  std::println("chinese: {}", j4);
  Person p4b = yuri::from_json<Person>(j4);
  std::println("chinese ok: name={}, hobbies=[{}]", p4b.name, joinStrings(p4b.hobbies));

  // 测试5：转义
  Person p5{ "Line1\nTab\tpQuote\"test", 35, 1.90, false, { "back\\slash" } };
  std::string j5 = yuri::to_json(p5);
  std::println("escape: {}", j5);
  Person p5b = yuri::from_json<Person>(j5);
  std::println("escape ok: name='{}', hobbies=['{}']", p5b.name, p5b.hobbies[0]);

  // 测试6：Address
  Address a{ "Beijing", "Chang'an Ave", 100000 };
  std::string ja = yuri::to_json(a);
  std::println("\naddress serialize: {}", ja);
  Address a2 = yuri::from_json<Address>(ja);
  std::println("address deserialize: city={}, street={}, zip={}", a2.city, a2.street, a2.zip_code);

  // 测试7：性能基准
  std::println("\n=== Benchmark ===");
  constexpr int kN = 100000;
  Person bp{ "BenchmarkUser", 99, 1.95, true, { "h1", "h2", "h3", "h4", "h5" } };
  for (int i = 0; i < 1000; ++i) {
    volatile auto s = yuri::to_json(bp);
  }

  auto t0 = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < kN; ++i) {
    volatile auto s = yuri::to_json(bp);
  }
  auto t1 = std::chrono::high_resolution_clock::now();
  auto ser_us = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
  std::println("serialize {}x: {} us ({} ns/op)", kN, ser_us, ser_us * 1000 / kN);

  std::string bj = yuri::to_json(bp);
  auto t2 = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < kN; ++i) {
    volatile auto v = yuri::from_json<Person>(bj);
  }
  auto t3 = std::chrono::high_resolution_clock::now();
  auto deser_us = std::chrono::duration_cast<std::chrono::microseconds>(t3 - t2).count();
  std::println("deserialize {}x: {} us ({} ns/op)", kN, deser_us, deser_us * 1000 / kN);

  return 0;
}
