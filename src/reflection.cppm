module;

export module yuri.json:reflection;

import std;
import :output;
import :parser;

// ============================================================================
// 反射序列化/反序列化
// ============================================================================

namespace yuri::json_detail {

/** @brief 编译期FNV-1a哈希 */
constexpr std::uint32_t fnv1aHash(std::string_view s) {
  std::uint32_t hash = 2166136261u;
  for (char c : s) {
    hash ^= static_cast<std::uint32_t>(c);
    hash *= 16777619u;
  }
  return hash;
}

/** @brief 从JSON直接填充反射结构体，使用编译期哈希快速匹配 */
template <typename T>
void fillStructDirect(T &obj, JsonParser &parser) {
  constexpr auto ctx = std::meta::access_context::current();
  static constexpr auto members =
    std::define_static_array(std::meta::nonstatic_data_members_of(^^T, ctx));

  if (!parser.match('{')) {
    return;
  }
  if (parser.match('}')) {
    return;
  }

  std::string_view key_buf;
  while (true) {
    parser.skipWs();
    std::uint32_t key_hash = 0;
    if (!parser.parseStringViewHash(key_buf, key_hash)) {
      return;
    }
    if (!parser.match(':')) {
      return;
    }

    // 使用编译期哈希快速匹配
    bool matched = false;
    template for (constexpr auto m : members) {
      if (!matched) {
        constexpr auto id = std::meta::identifier_of(m);
        constexpr std::uint32_t member_hash = fnv1aHash(id);
        if (key_hash == member_hash && key_buf == id) {
          matched = true;
          constexpr auto mt = std::meta::type_of(m);
          auto &field = obj.[:m:];
          if constexpr (std::is_same_v<typename[:mt:], std::string>) {
            if (!parser.parseDirectString(field)) {
              parser.consumeValue();
            }
          } else if constexpr (std::is_same_v<typename[:mt:], int>) {
            if (!parser.parseDirectNumber(field)) {
              parser.consumeValue();
            }
          } else if constexpr (std::is_same_v<typename[:mt:], long long>) {
            if (!parser.parseDirectNumber(field)) {
              parser.consumeValue();
            }
          } else if constexpr (std::is_same_v<typename[:mt:], double>) {
            if (!parser.parseDirectNumber(field)) {
              parser.consumeValue();
            }
          } else if constexpr (std::is_same_v<typename[:mt:], float>) {
            if (!parser.parseDirectNumber(field)) {
              parser.consumeValue();
            }
          } else if constexpr (std::is_same_v<typename[:mt:], bool>) {
            if (!parser.parseDirectBool(field)) {
              parser.consumeValue();
            }
          } else if constexpr (std::is_same_v<typename[:mt:], std::vector<std::string>>) {
            if (parser.match('[')) {
              field.clear();
              field.reserve(8);
              if (!parser.match(']')) {
                std::string elem;
                while (true) {
                  if (parser.parseDirectString(elem)) {
                    field.push_back(std::move(elem));
                  } else {
                    parser.consumeValue();
                  }
                  if (!parser.match(',')) {
                    break;
                  }
                }
                parser.match(']');
              }
            } else {
              parser.consumeValue();
            }
          } else if constexpr (std::is_same_v<typename[:mt:], std::vector<int>>) {
            if (parser.match('[')) {
              field.clear();
              field.reserve(8);
              if (!parser.match(']')) {
                int elem = 0;
                while (true) {
                  if (parser.parseDirectNumber(elem)) {
                    field.push_back(elem);
                  } else {
                    parser.consumeValue();
                  }
                  if (!parser.match(',')) {
                    break;
                  }
                }
                parser.match(']');
              }
            } else {
              parser.consumeValue();
            }
          } else if constexpr (std::is_same_v<typename[:mt:], std::vector<double>>) {
            if (parser.match('[')) {
              field.clear();
              field.reserve(32);
              if (!parser.match(']')) {
                double elem = 0.0;
                while (true) {
                  if (parser.parseDirectNumber(elem)) {
                    field.push_back(elem);
                  } else {
                    parser.consumeValue();
                  }
                  if (!parser.match(',')) {
                    break;
                  }
                }
                parser.match(']');
              }
            } else {
              parser.consumeValue();
            }
          } else {
            parser.consumeValue();
          }
        }
      }
    }

    if (!matched) {
      parser.consumeValue();
    }
    if (!parser.match(',')) {
      break;
    }
  }
  parser.match('}');
}

} // namespace yuri::json_detail

// ============================================================================
// 模块导出实现
// ============================================================================

export namespace yuri {

template <typename T>
std::string to_json(const T &value) {
  std::string out;
  out.reserve(512);
  out += '{';
  constexpr auto ctx = std::meta::access_context::current();
  static constexpr auto members =
    std::define_static_array(std::meta::nonstatic_data_members_of(^^T, ctx));
  bool first = true;
  template for (constexpr auto m : members) {
    if (!first) {
      out += ',';
    }
    first = false;
    json_detail::writeKey(out, std::meta::identifier_of(m));
    out += ':';
    constexpr auto mt = std::meta::type_of(m);
    if constexpr (std::is_same_v<typename[:mt:], std::string>) {
      json_detail::writeEscapedString(out, value.[:m:]);
    } else if constexpr (std::is_same_v<typename[:mt:], int>
                         || std::is_same_v<typename[:mt:], long long>) {
      json_detail::writeInt(out, value.[:m:]);
    } else if constexpr (std::is_same_v<typename[:mt:], double>
                         || std::is_same_v<typename[:mt:], float>) {
      json_detail::writeFloat(out, value.[:m:]);
    } else if constexpr (std::is_same_v<typename[:mt:], bool>) {
      out += value.[:m:] ? "true" : "false";
    } else if constexpr (std::is_same_v<typename[:mt:], std::vector<std::string>>) {
      json_detail::writeStringArray(out, value.[:m:]);
    } else if constexpr (std::is_same_v<typename[:mt:], std::vector<int>>) {
      json_detail::writeIntArray(out, value.[:m:]);
    } else if constexpr (std::is_same_v<typename[:mt:], std::vector<double>>) {
      json_detail::writeDoubleArray(out, value.[:m:]);
    }
  }
  out += '}';
  return out;
}

template <typename T>
T from_json(std::string_view json_str) {
  T obj{};
  json_detail::JsonParser parser(json_str);
  json_detail::fillStructDirect(obj, parser);
  return obj;
}

} // namespace yuri
