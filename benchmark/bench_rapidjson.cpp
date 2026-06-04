#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include "bench_common.hpp"

/** @brief rapidjson benchmark主函数 */
int main() {
  std::println("=== rapidjson Benchmark ===\n");

  std::vector<BenchResult> results;

  // SmallObj
  {
    auto obj = makeSmallObj();
    auto json_str = makeSmallJson();
    auto ser = runSerializeBench("Small", [&] {
      rapidjson::StringBuffer buffer;
      rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
      writer.StartObject();
      writer.Key("name");
      writer.String(obj.name.c_str(), static_cast<rapidjson::SizeType>(obj.name.size()));
      writer.Key("id");
      writer.Int(obj.id);
      writer.Key("score");
      writer.Double(obj.score);
      writer.Key("active");
      writer.Bool(obj.active);
      writer.EndObject();
      return std::string(buffer.GetString());
    });
    auto deser = runDeserializeBench("Small", [&] {
      rapidjson::Document doc;
      doc.Parse(json_str.c_str());
      SmallObj o;
      o.name = doc["name"].GetString();
      o.id = doc["id"].GetInt();
      o.score = doc["score"].GetDouble();
      o.active = doc["active"].GetBool();
      return o;
    });
    results.push_back({ "SmallObj", ser, deser });
  }

  // MediumObj
  {
    auto obj = makeMediumObj();
    auto json_str = makeMediumJson();
    auto ser = runSerializeBench("Medium", [&] {
      rapidjson::StringBuffer buffer;
      rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
      writer.StartObject();
      writer.Key("title");
      writer.String(obj.title.c_str(), static_cast<rapidjson::SizeType>(obj.title.size()));
      writer.Key("count");
      writer.Int(obj.count);
      writer.Key("ratio");
      writer.Double(obj.ratio);
      writer.Key("enabled");
      writer.Bool(obj.enabled);
      writer.Key("tags");
      writer.StartArray();
      for (const auto &tag : obj.tags) {
        writer.String(tag.c_str(), static_cast<rapidjson::SizeType>(tag.size()));
      }
      writer.EndArray();
      writer.Key("scores");
      writer.StartArray();
      for (auto score : obj.scores) {
        writer.Int(score);
      }
      writer.EndArray();
      writer.EndObject();
      return std::string(buffer.GetString());
    });
    auto deser = runDeserializeBench("Medium", [&] {
      rapidjson::Document doc;
      doc.Parse(json_str.c_str());
      MediumObj o;
      o.title = doc["title"].GetString();
      o.count = doc["count"].GetInt();
      o.ratio = doc["ratio"].GetDouble();
      o.enabled = doc["enabled"].GetBool();
      for (auto &v : doc["tags"].GetArray()) {
        o.tags.push_back(v.GetString());
      }
      for (auto &v : doc["scores"].GetArray()) {
        o.scores.push_back(v.GetInt());
      }
      return o;
    });
    results.push_back({ "MediumObj", ser, deser });
  }

  // ComplexObj
  {
    auto obj = makeComplexObj();
    auto json_str = makeComplexJson();
    auto ser = runSerializeBench("Complex", [&] {
      rapidjson::StringBuffer buffer;
      rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
      writer.StartObject();
      writer.Key("name");
      writer.String(obj.name.c_str(), static_cast<rapidjson::SizeType>(obj.name.size()));
      writer.Key("age");
      writer.Int(obj.age);
      writer.Key("height");
      writer.Double(obj.height);
      writer.Key("is_student");
      writer.Bool(obj.is_student);
      writer.Key("hobbies");
      writer.StartArray();
      for (const auto &hobby : obj.hobbies) {
        writer.String(hobby.c_str(), static_cast<rapidjson::SizeType>(hobby.size()));
      }
      writer.EndArray();
      writer.Key("metrics");
      writer.StartArray();
      for (auto metric : obj.metrics) {
        writer.Double(metric);
      }
      writer.EndArray();
      writer.EndObject();
      return std::string(buffer.GetString());
    });
    auto deser = runDeserializeBench("Complex", [&] {
      rapidjson::Document doc;
      doc.Parse(json_str.c_str());
      ComplexObj o;
      o.name = doc["name"].GetString();
      o.age = doc["age"].GetInt();
      o.height = doc["height"].GetDouble();
      o.is_student = doc["is_student"].GetBool();
      for (auto &v : doc["hobbies"].GetArray()) {
        o.hobbies.push_back(v.GetString());
      }
      for (auto &v : doc["metrics"].GetArray()) {
        o.metrics.push_back(v.GetDouble());
      }
      return o;
    });
    results.push_back({ "ComplexObj", ser, deser });
  }

  // LargeObj
  {
    auto obj = makeLargeObj();
    auto json_str = makeLargeJson();
    auto ser = runSerializeBench("Large", [&] {
      rapidjson::StringBuffer buffer;
      rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
      writer.StartObject();
      writer.Key("title");
      writer.String(obj.title.c_str(), static_cast<rapidjson::SizeType>(obj.title.size()));
      writer.Key("version");
      writer.Int(obj.version);
      writer.Key("debug");
      writer.Bool(obj.debug);
      writer.Key("tags");
      writer.StartArray();
      for (const auto &tag : obj.tags)
        writer.String(tag.c_str(), static_cast<rapidjson::SizeType>(tag.size()));
      writer.EndArray();
      writer.Key("indices");
      writer.StartArray();
      for (auto v : obj.indices)
        writer.Int(v);
      writer.EndArray();
      writer.Key("values");
      writer.StartArray();
      for (auto v : obj.values)
        writer.Double(v);
      writer.EndArray();
      writer.EndObject();
      return std::string(buffer.GetString());
    });
    auto deser = runDeserializeBench("Large", [&] {
      rapidjson::Document doc;
      doc.Parse(json_str.c_str());
      LargeObj o;
      o.title = doc["title"].GetString();
      o.version = doc["version"].GetInt();
      o.debug = doc["debug"].GetBool();
      for (auto &v : doc["tags"].GetArray())
        o.tags.push_back(v.GetString());
      for (auto &v : doc["indices"].GetArray())
        o.indices.push_back(v.GetInt());
      for (auto &v : doc["values"].GetArray())
        o.values.push_back(v.GetDouble());
      return o;
    });
    results.push_back({ "LargeObj(~15KB)", ser, deser });
  }

  printResult("rapidjson", results);
  return 0;
}
