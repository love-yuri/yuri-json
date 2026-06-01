/*
 * @Author: love-yuri yuri2078170658@gmail.com
 * @Date: 2026-05-29 10:58:25
 * @LastEditTime: 2026-06-01 14:21:45
 * @Description:
 */
#include <simdjson.h>
#include "bench_common.hpp"

/** @brief simdjson benchmark主函数（仅反序列化，simdjson无序列化器） */
int main() {
  std::println("=== simdjson Benchmark ===\n");
  std::println("注: simdjson仅提供解析器，无序列化功能\n");

  std::vector<BenchResult> results;
  simdjson::ondemand::parser parser;

  // SmallObj
  {
    auto json_str = makeSmallJson();
    auto deser = runDeserializeBench("Small", [&] {
      simdjson::padded_string padded(json_str);
      auto doc = parser.iterate(padded);
      SmallObj o;
      o.name = std::string(doc["name"].get_string().value());
      o.id = static_cast<int>(doc["id"].get_int64().value());
      o.score = doc["score"].get_double().value();
      o.active = doc["active"].get_bool().value();
      return o;
    });
    results.push_back({ "SmallObj", -1.0, deser });
  }

  // MediumObj
  {
    auto json_str = makeMediumJson();
    auto deser = runDeserializeBench("Medium", [&] {
      simdjson::padded_string padded(json_str);
      auto doc = parser.iterate(padded);
      MediumObj o;
      o.title = std::string(doc["title"].get_string().value());
      o.count = static_cast<int>(doc["count"].get_int64().value());
      o.ratio = doc["ratio"].get_double().value();
      o.enabled = doc["enabled"].get_bool().value();
      for (auto tag : doc["tags"].get_array()) {
        o.tags.push_back(std::string(tag.get_string().value()));
      }
      for (auto score : doc["scores"].get_array()) {
        o.scores.push_back(static_cast<int>(score.get_int64().value()));
      }
      return o;
    });
    results.push_back({ "MediumObj", -1.0, deser });
  }

  // ComplexObj
  {
    auto json_str = makeComplexJson();
    auto deser = runDeserializeBench("Complex", [&] {
      simdjson::padded_string padded(json_str);
      auto doc = parser.iterate(padded);
      ComplexObj o;
      o.name = std::string(doc["name"].get_string().value());
      o.age = static_cast<int>(doc["age"].get_int64().value());
      o.height = doc["height"].get_double().value();
      o.is_student = doc["is_student"].get_bool().value();
      for (auto hobby : doc["hobbies"].get_array()) {
        o.hobbies.push_back(std::string(hobby.get_string().value()));
      }
      for (auto metric : doc["metrics"].get_array()) {
        o.metrics.push_back(metric.get_double().value());
      }
      return o;
    });
    results.push_back({ "ComplexObj", -1.0, deser });
  }

  // 手动输出（simdjson无序列化）
  std::println("=== simdjson ===");
  std::println("{:<25} {:>12} {:>12}", "Benchmark", "Serialize", "Deserialize");
  std::println("{:<25} {:>12} {:>12}", "---------", "---------", "-----------");
  for (const auto &r : results) {
    std::println("{:<25} {:>12} {:>9.3f} us", r.name, "N/A", r.deserialize_us);
  }
  std::println("");

  return 0;
}
