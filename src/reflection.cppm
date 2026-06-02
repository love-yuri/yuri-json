module;

export module yuri.json:reflection;

import std;
import :output;
import :parser;

// ============================================================================
// 反射序列化/反序列化（极致优化版 - 编译期内联）
// ============================================================================

namespace yuri::json_detail {

/** @brief 估算单个字段序列化后的大小上界 */
template <typename FieldType>
std::size_t estimateValueSize(const FieldType &field) {
  if constexpr (std::is_same_v<FieldType, std::string>) {
    return 2 + field.size() * 6;
  } else if constexpr (std::is_same_v<FieldType, int> || std::is_same_v<FieldType, long long>) {
    return intSize(field);
  } else if constexpr (std::is_same_v<FieldType, double> || std::is_same_v<FieldType, float>) {
    return kFloatSizeUpperBound;
  } else if constexpr (std::is_same_v<FieldType, bool>) {
    return field ? 4 : 5;
  } else if constexpr (std::is_same_v<FieldType, std::vector<std::string>>) {
    std::size_t size = 2;
    for (const auto &elem : field) {
      size += 2 + elem.size() * 6 + 1;
    }
    return field.empty() ? size : size - 1;
  } else if constexpr (std::is_same_v<FieldType, std::vector<int>>) {
    return 2 + field.size() * 12 - (field.empty() ? 0 : 1);
  } else if constexpr (std::is_same_v<FieldType, std::vector<double>>) {
    return 2 + field.size() * (kFloatSizeUpperBound + 1) - (field.empty() ? 0 : 1);
  } else {
    return 0;
  }
}

/** @brief 基于反射估算结构体序列化后的大小上界 */
template <typename T>
std::size_t estimateStructSize(const T &value) {
  constexpr auto ctx = std::meta::access_context::current();
  static constexpr auto members =
    std::define_static_array(std::meta::nonstatic_data_members_of(^^T, ctx));
  std::size_t size = 2;
  bool first = true;
  template for (constexpr auto m : members) {
    if (!first) {
      ++size;
    }
    first = false;
    constexpr auto id = std::meta::identifier_of(m);
    size += id.size() + 3;
    size += estimateValueSize(value.[:m:]);
  }
  return size;
}

template <typename Buffer, typename FieldType>
void writeValueDirect(Buffer &buf, const FieldType &field) {
  if constexpr (std::is_same_v<FieldType, std::string>) {
    writeEscapedStringBuf(buf, field);
  } else if constexpr (std::is_same_v<FieldType, int> || std::is_same_v<FieldType, long long>) {
    writeIntBuf(buf, field);
  } else if constexpr (std::is_same_v<FieldType, double> || std::is_same_v<FieldType, float>) {
    writeFloatBuf(buf, field);
  } else if constexpr (std::is_same_v<FieldType, bool>) {
    if (field) {
      buf.write("true", 4);
    } else {
      buf.write("false", 5);
    }
  } else if constexpr (std::is_same_v<FieldType, std::vector<std::string>>) {
    buf.push('[');
    for (std::size_t i = 0; i < field.size(); ++i) {
      if (i != 0) {
        buf.push(',');
      }
      writeEscapedStringBuf(buf, field[i]);
    }
    buf.push(']');
  } else if constexpr (std::is_same_v<FieldType, std::vector<int>>) {
    buf.push('[');
    for (std::size_t i = 0; i < field.size(); ++i) {
      if (i != 0) {
        buf.push(',');
      }
      writeIntBuf(buf, field[i]);
    }
    buf.push(']');
  } else if constexpr (std::is_same_v<FieldType, std::vector<double>>) {
    buf.push('[');
    for (std::size_t i = 0; i < field.size(); ++i) {
      if (i != 0) {
        buf.push(',');
      }
      writeFloatBuf(buf, field[i]);
    }
    buf.push(']');
  }
}

template <typename Buffer, typename T>
void writeStructDirect(Buffer &buf, const T &value) {
  buf.push('{');
  constexpr auto ctx = std::meta::access_context::current();
  static constexpr auto members =
    std::define_static_array(std::meta::nonstatic_data_members_of(^^T, ctx));
  bool first = true;
  template for (constexpr auto m : members) {
    if (!first) {
      buf.push(',');
    }
    first = false;
    writeKeyBuf(buf, std::meta::identifier_of(m));
    buf.push(':');
    writeValueDirect(buf, value.[:m:]);
  }
  buf.push('}');
}

constexpr std::uint32_t fnv1aHash(std::string_view s) {
  std::uint32_t hash = 2166136261u;
  for (char c : s) {
    hash ^= static_cast<std::uint32_t>(c);
    hash *= 16777619u;
  }
  return hash;
}

/** @brief 解析单个字段值 */
template <typename FieldType>
void parseFieldValue(FieldType &field, JsonParser &parser) {
  if constexpr (std::is_same_v<FieldType, std::string>) {
    if (__builtin_expect(!parser.parseDirectString(field), false)) {
      parser.consumeValue();
    }
  } else if constexpr (std::is_same_v<FieldType, int> || std::is_same_v<FieldType, long long>) {
    if (__builtin_expect(!parser.parseDirectNumber(field), false)) {
      parser.consumeValue();
    }
  } else if constexpr (std::is_same_v<FieldType, double> || std::is_same_v<FieldType, float>) {
    if (__builtin_expect(!parser.parseDirectNumber(field), false)) {
      parser.consumeValue();
    }
  } else if constexpr (std::is_same_v<FieldType, bool>) {
    if (__builtin_expect(!parser.parseDirectBool(field), false)) {
      parser.consumeValue();
    }
  } else if constexpr (std::is_same_v<FieldType, std::vector<std::string>>) {
    if (__builtin_expect(parser.match('['), true)) {
      field.clear();
      field.reserve(8);
      if (__builtin_expect(!parser.match(']'), true)) {
        std::string elem;
        while (true) {
          if (__builtin_expect(parser.parseDirectString(elem), true)) {
            field.push_back(std::move(elem));
          } else {
            parser.consumeValue();
          }
          if (__builtin_expect(parser.matchNoWs(','), true)) {
            if (__builtin_expect(parser.nextIsWs(), false)) {
              parser.skipWs();
            }
            continue;
          }
          if (__builtin_expect(parser.nextIsWs(), false)) {
            parser.skipWs();
          }
          break;
        }
        parser.matchNoWs(']');
      }
    } else {
      parser.consumeValue();
    }
  } else if constexpr (std::is_same_v<FieldType, std::vector<int>>) {
    if (__builtin_expect(parser.parseIntArray(field), true)) {
      // 成功
    } else {
      parser.consumeValue();
    }
  } else if constexpr (std::is_same_v<FieldType, std::vector<double>>) {
    if (__builtin_expect(parser.parseDoubleArray(field), true)) {
      // 成功
    } else {
      parser.consumeValue();
    }
  } else {
    parser.consumeValue();
  }
}

template <typename T>
bool fillStructOrdered(T &obj, JsonParser &parser) {
  constexpr auto ctx = std::meta::access_context::current();
  static constexpr auto members =
    std::define_static_array(std::meta::nonstatic_data_members_of(^^T, ctx));

  const char *start = parser.mark();
  if (__builtin_expect(!parser.match('{'), false)) {
    parser.rewind(start);
    return false;
  }
  if (__builtin_expect(parser.match('}'), false)) {
    return true;
  }

  bool first = true;
  template for (constexpr auto m : members) {
    if (!first && __builtin_expect(!parser.matchNoWsOrWs(','), false)) {
      parser.rewind(start);
      return false;
    }
    first = false;
    if (__builtin_expect(parser.nextIsWs(), false)) {
      parser.skipWs();
    }
    if (__builtin_expect(!parser.matchKeyNoWs(std::meta::identifier_of(m)), false)) {
      parser.rewind(start);
      return false;
    }
    if (__builtin_expect(!parser.matchNoWsOrWs(':'), false)) {
      parser.rewind(start);
      return false;
    }
    parseFieldValue(obj.[:m:], parser);
  }
  if (__builtin_expect(!parser.matchNoWsOrWs('}'), false)) {
    parser.rewind(start);
    return false;
  }
  return true;
}

/** @brief 从JSON直接填充反射结构体，使用编译期哈希快速匹配 */
template <typename T>
void fillStructDirect(T &obj, JsonParser &parser) {
  if (__builtin_expect(fillStructOrdered(obj, parser), true)) {
    return;
  }

  constexpr auto ctx = std::meta::access_context::current();
  static constexpr auto members =
    std::define_static_array(std::meta::nonstatic_data_members_of(^^T, ctx));

  if (__builtin_expect(!parser.match('{'), false)) {
    return;
  }
  if (__builtin_expect(parser.match('}'), false)) {
    return;
  }

  std::string_view key_buf;
  while (true) {
    parser.skipWs();
    std::uint32_t key_hash = 0;
    if (__builtin_expect(!parser.parseStringViewHash(key_buf, key_hash), false)) {
      return;
    }
    if (__builtin_expect(!parser.match(':'), false)) {
      return;
    }

    // 编译期展开：每个字段生成一个 if(hash匹配) { 解析字段; }
    // 这就是 std::meta 的编译期内联代码
    bool matched = false;
    template for (constexpr auto m : members) {
      if (!matched) {
        constexpr auto id = std::meta::identifier_of(m);
        constexpr std::uint32_t member_hash = fnv1aHash(id);
        if (__builtin_expect(key_hash == member_hash && key_buf == id, true)) {
          matched = true;
          parseFieldValue(obj.[:m:], parser);
        }
      }
    }

    if (__builtin_expect(!matched, false)) {
      parser.consumeValue();
    }
    if (__builtin_expect(!parser.match(','), false)) {
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
  out.resize_and_overwrite(json_detail::estimateStructSize(value), [&](char *data, std::size_t) {
    json_detail::DirectBuffer buf(data);
    json_detail::writeStructDirect(buf, value);
    return buf.size();
  });
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
