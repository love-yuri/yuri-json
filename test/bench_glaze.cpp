/*
 * @Author: love-yuri yuri2078170658@gmail.com
 * @Date: 2026-05-29 10:30:39
 * @LastEditTime: 2026-06-01 14:22:44
 * @Description:
 */
#include <glaze/glaze.hpp>
#include "bench_common.hpp"

// ============================================================================
// glz::meta特化定义
// ============================================================================

template <>
struct glz::meta<SmallObj> {
  using T = SmallObj;
  static constexpr auto value =
    glz::object("name", &T::name, "id", &T::id, "score", &T::score, "active", &T::active);
};

template <>
struct glz::meta<MediumObj> {
  using T = MediumObj;
  static constexpr auto value = glz::object(
    "title",
    &T::title,
    "count",
    &T::count,
    "ratio",
    &T::ratio,
    "enabled",
    &T::enabled,
    "tags",
    &T::tags,
    "scores",
    &T::scores
  );
};

template <>
struct glz::meta<ComplexObj> {
  using T = ComplexObj;
  static constexpr auto value = glz::object(
    "name",
    &T::name,
    "age",
    &T::age,
    "height",
    &T::height,
    "is_student",
    &T::is_student,
    "hobbies",
    &T::hobbies,
    "metrics",
    &T::metrics
  );
};

/** @brief glaze benchmark主函数 */
int main() {
  std::println("=== glaze Benchmark ===\n");

  std::vector<BenchResult> results;

  // SmallObj
  {
    auto obj = makeSmallObj();
    auto json_str = makeSmallJson();
    auto ser = runSerializeBench("Small", [&] { return glz::write_json(obj).value(); });
    auto deser = runDeserializeBench("Small", [&] {
      SmallObj o{};
      glz::read_json(o, json_str);
      return o;
    });
    results.push_back({ "SmallObj", ser, deser });
  }

  // MediumObj
  {
    auto obj = makeMediumObj();
    auto json_str = makeMediumJson();
    auto ser = runSerializeBench("Medium", [&] { return glz::write_json(obj).value(); });
    auto deser = runDeserializeBench("Medium", [&] {
      MediumObj o{};
      glz::read_json(o, json_str);
      return o;
    });
    results.push_back({ "MediumObj", ser, deser });
  }

  // ComplexObj
  {
    auto obj = makeComplexObj();
    auto json_str = makeComplexJson();
    auto ser = runSerializeBench("Complex", [&] { return glz::write_json(obj).value(); });
    auto deser = runDeserializeBench("Complex", [&] {
      ComplexObj o{};
      glz::read_json(o, json_str);
      return o;
    });
    results.push_back({ "ComplexObj", ser, deser });
  }

  printResult("glaze", results);
  return 0;
}
