/*
 * @Author: love-yuri yuri2078170658@gmail.com
 * @Date: 2026-05-29 10:29:59
 * @LastEditTime: 2026-06-01 14:22:11
 * @Description:
 */
#include <nlohmann/json.hpp>
#include "bench_common.hpp"

/** @brief nlohmann/json benchmark主函数 */
int main() {
  std::println("=== nlohmann/json Benchmark ===\n");

  std::vector<BenchResult> results;

  // SmallObj
  {
    auto obj = makeSmallObj();
    auto json_str = makeSmallJson();
    auto ser = runSerializeBench("Small", [&] {
      nlohmann::json j;
      j["name"] = obj.name;
      j["id"] = obj.id;
      j["score"] = obj.score;
      j["active"] = obj.active;
      return j.dump();
    });
    auto deser = runDeserializeBench("Small", [&] {
      auto j = nlohmann::json::parse(json_str);
      SmallObj o;
      o.name = j["name"].get<std::string>();
      o.id = j["id"].get<int>();
      o.score = j["score"].get<double>();
      o.active = j["active"].get<bool>();
      return o;
    });
    results.push_back({ "SmallObj", ser, deser });
  }

  // MediumObj
  {
    auto obj = makeMediumObj();
    auto json_str = makeMediumJson();
    auto ser = runSerializeBench("Medium", [&] {
      nlohmann::json j;
      j["title"] = obj.title;
      j["count"] = obj.count;
      j["ratio"] = obj.ratio;
      j["enabled"] = obj.enabled;
      j["tags"] = obj.tags;
      j["scores"] = obj.scores;
      return j.dump();
    });
    auto deser = runDeserializeBench("Medium", [&] {
      auto j = nlohmann::json::parse(json_str);
      MediumObj o;
      o.title = j["title"].get<std::string>();
      o.count = j["count"].get<int>();
      o.ratio = j["ratio"].get<double>();
      o.enabled = j["enabled"].get<bool>();
      o.tags = j["tags"].get<std::vector<std::string>>();
      o.scores = j["scores"].get<std::vector<int>>();
      return o;
    });
    results.push_back({ "MediumObj", ser, deser });
  }

  // ComplexObj
  {
    auto obj = makeComplexObj();
    auto json_str = makeComplexJson();
    auto ser = runSerializeBench("Complex", [&] {
      nlohmann::json j;
      j["name"] = obj.name;
      j["age"] = obj.age;
      j["height"] = obj.height;
      j["is_student"] = obj.is_student;
      j["hobbies"] = obj.hobbies;
      j["metrics"] = obj.metrics;
      return j.dump();
    });
    auto deser = runDeserializeBench("Complex", [&] {
      auto j = nlohmann::json::parse(json_str);
      ComplexObj o;
      o.name = j["name"].get<std::string>();
      o.age = j["age"].get<int>();
      o.height = j["height"].get<double>();
      o.is_student = j["is_student"].get<bool>();
      o.hobbies = j["hobbies"].get<std::vector<std::string>>();
      o.metrics = j["metrics"].get<std::vector<double>>();
      return o;
    });
    results.push_back({ "ComplexObj", ser, deser });
  }

  printResult("nlohmann", results);
  return 0;
}
