module;

export module yuri.json:reflection;

import std;
import :output;
import :parser;

// ============================================================================
// 反射序列化/反序列化（编译期内联）
// ============================================================================

export namespace yuri {

struct JsonDiagnostics {
  std::size_t context_chars = 32; // 错误片段上下文字符数
};

inline constexpr JsonDiagnostics kWithDiagnostics{}; // 默认诊断配置

struct JsonErrorContext {
  std::size_t offset{};       // 错误字节偏移
  std::size_t line{};         // 错误行号
  std::size_t column{};       // 错误列号
  std::string expected;       // 期望内容
  std::string found;          // 实际内容
  std::string snippet;        // 错误附近片段
  std::size_t caret_column{}; // 指示符所在列
};

class JsonParseError : public std::runtime_error {
public:
  JsonParseError(std::string message, JsonErrorContext context) :
    std::runtime_error(std::move(message)), context_(std::move(context)) {
  }

  const JsonErrorContext &context() const noexcept {
    return context_;
  }

private:
  JsonErrorContext context_; // 错误上下文
};

} // namespace yuri

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
bool parseFieldValue(FieldType &field, JsonParser &parser, std::string_view field_name = {}) {
  bool ok = false;
  if constexpr (std::is_same_v<FieldType, std::string>) {
    ok = parser.parseDirectString(field);
  } else if constexpr (std::is_same_v<FieldType, int> || std::is_same_v<FieldType, long long>) {
    ok = parser.parseDirectNumber(field);
  } else if constexpr (std::is_same_v<FieldType, double> || std::is_same_v<FieldType, float>) {
    ok = parser.parseDirectNumber(field);
  } else if constexpr (std::is_same_v<FieldType, bool>) {
    ok = parser.parseDirectBool(field);
  } else if constexpr (std::is_same_v<FieldType, std::vector<std::string>>) {
    if (__builtin_expect(!parser.match('['), false)) {
      ok = false;
    } else {
      field.clear();
      field.reserve(8);
      if (__builtin_expect(parser.match(']'), false)) {
        ok = true;
      } else {
        while (true) {
          std::string elem;
          if (__builtin_expect(!parser.parseDirectString(elem), false)) {
            ok = false;
            break;
          }
          field.push_back(std::move(elem));
          if (__builtin_expect(parser.matchNoWs(','), true)) {
            if (__builtin_expect(parser.nextIsWs(), false)) {
              parser.skipWs();
            }
            continue;
          }
          if (__builtin_expect(parser.nextIsWs(), false)) {
            parser.skipWs();
          }
          if (__builtin_expect(!parser.matchNoWs(']'), false)) {
            parser.recordError("',' or ']' after array element", {}, field_name, 3);
            ok = false;
            break;
          }
          ok = true;
          break;
        }
      }
    }
  } else if constexpr (std::is_same_v<FieldType, std::vector<int>>) {
    ok = parser.parseIntArray(field);
  } else if constexpr (std::is_same_v<FieldType, std::vector<double>>) {
    ok = parser.parseDoubleArray(field);
  } else {
    ok = parser.consumeValue();
  }
  if (!ok && !field_name.empty()) {
    parser.annotateErrorField(field_name);
  }
  return ok;
}

std::string describeExpected(const JsonErrorRecord &record) {
  std::string expected(record.expected.empty() ? "valid JSON" : record.expected);
  if (!record.field.empty()) {
    expected += " for field '";
    expected += record.field;
    expected += "'";
  }
  return expected;
}

std::string describeFound(std::string_view json_str, const char *pos) {
  const char *begin = json_str.data();
  const char *end = begin + json_str.size();
  if (pos >= end) {
    return "end of input";
  }
  if (*pos == '\n') {
    return "newline";
  }
  if (*pos == '\r') {
    return "carriage return";
  }
  if (*pos == '\t') {
    return "tab";
  }
  return std::string(pos, 1);
}

yuri::JsonErrorContext buildErrorContext(
  std::string_view json_str,
  const JsonErrorRecord &record,
  yuri::JsonDiagnostics diagnostics
) {
  const char *begin = json_str.data();
  const char *end = begin + json_str.size();
  const char *pos = record.has ? record.pos : begin;
  if (pos < begin) {
    pos = begin;
  }
  if (pos > end) {
    pos = end;
  }

  yuri::JsonErrorContext context;
  context.offset = static_cast<std::size_t>(pos - begin);
  context.line = 1;
  context.column = 1;

  const char *line_begin = begin;
  for (const char *it = begin; it < pos; ++it) {
    if (*it == '\n') {
      ++context.line;
      context.column = 1;
      line_begin = it + 1;
    } else {
      ++context.column;
    }
  }

  const char *line_end = pos;
  while (line_end < end && *line_end != '\n' && *line_end != '\r') {
    ++line_end;
  }

  const char *snippet_begin = line_begin;
  auto prefix_len = static_cast<std::size_t>(pos - line_begin);
  if (prefix_len > diagnostics.context_chars) {
    snippet_begin = pos - diagnostics.context_chars;
  }
  const char *snippet_end = line_end;
  auto suffix_len = static_cast<std::size_t>(line_end - pos);
  if (suffix_len > diagnostics.context_chars) {
    snippet_end = pos + diagnostics.context_chars;
  }

  context.snippet.assign(snippet_begin, snippet_end);
  context.caret_column = static_cast<std::size_t>(pos - snippet_begin) + 1;
  context.expected = describeExpected(record);
  context.found = record.found.empty() ? describeFound(json_str, pos) : std::string(record.found);
  return context;
}

yuri::JsonParseError buildParseError(
  std::string_view json_str,
  const JsonErrorRecord &record,
  yuri::JsonDiagnostics diagnostics
) {
  static constexpr std::string_view kReset = "\x1b[0m";
  static constexpr std::string_view kRed = "\x1b[31m";
  static constexpr std::string_view kYellow = "\x1b[33m";
  static constexpr std::string_view kCyan = "\x1b[36m";
  static constexpr std::string_view kDim = "\x1b[2m";
  static constexpr std::string_view kBold = "\x1b[1m";

  auto context = buildErrorContext(json_str, record, diagnostics);
  std::string message;
  message.reserve(context.snippet.size() + context.expected.size() + context.found.size() + 160);
  message += kBold;
  message += kRed;
  message += "JSON parse error";
  message += kReset;
  message += "\n  ";
  message += kCyan;
  message += "-->";
  message += kReset;
  message += " line ";
  message += std::to_string(context.line);
  message += ", column ";
  message += std::to_string(context.column);
  message += "\n  ";
  message += kYellow;
  message += "expected";
  message += kReset;
  message += ": ";
  message += context.expected;
  message += "\n  ";
  message += kYellow;
  message += "found";
  message += kReset;
  message += ": ";
  message += context.found;
  if (!context.snippet.empty()) {
    message += "\n\n  ";
    message += kDim;
    message += "| ";
    message += kReset;
    message += context.snippet;
    message += "\n  ";
    message += kDim;
    message += "| ";
    message += kReset;
    message.append(context.caret_column > 0 ? context.caret_column - 1 : 0, ' ');
    message += kRed;
    message += kBold;
    message += '^';
    message += kReset;
  }
  return yuri::JsonParseError(std::move(message), std::move(context));
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
    if (__builtin_expect(!parseFieldValue(obj.[:m:], parser, std::meta::identifier_of(m)), false)) {
      parser.rewind(start);
      return false;
    }
  }
  if (__builtin_expect(!parser.matchNoWsOrWs('}'), false)) {
    parser.rewind(start);
    return false;
  }
  return true;
}

/** @brief 从JSON直接填充反射结构体，使用编译期哈希快速匹配 */
template <typename T>
bool fillStructDirect(T &obj, JsonParser &parser) {
  if (parser.hasErrorRecorder()) {
    auto suppress = parser.suppressErrors();
    if (__builtin_expect(fillStructOrdered(obj, parser), true)) {
      return true;
    }
  } else if (__builtin_expect(fillStructOrdered(obj, parser), true)) {
    return true;
  }

  constexpr auto ctx = std::meta::access_context::current();
  static constexpr auto members =
    std::define_static_array(std::meta::nonstatic_data_members_of(^^T, ctx));

  if (__builtin_expect(!parser.match('{'), false)) {
    return false;
  }
  if (__builtin_expect(parser.match('}'), false)) {
    return true;
  }

  std::string_view key_buf;
  while (true) {
    parser.skipWs();
    std::uint32_t key_hash = 0;
    if (__builtin_expect(!parser.parseStringViewHash(key_buf, key_hash), false)) {
      return false;
    }
    if (__builtin_expect(!parser.match(':'), false)) {
      return false;
    }

    // 编译期展开：每个字段生成一个 if(hash匹配) { 解析字段; }
    // 这就是 std::meta 的编译期内联代码
    bool matched = false;
    bool parsed = false;
    template for (constexpr auto m : members) {
      if (!matched) {
        constexpr auto id = std::meta::identifier_of(m);
        constexpr std::uint32_t member_hash = fnv1aHash(id);
        if (__builtin_expect(key_hash == member_hash && key_buf == id, true)) {
          matched = true;
          parsed = parseFieldValue(obj.[:m:], parser, id);
        }
      }
    }

    if (__builtin_expect(!matched, false)) {
      parsed = parser.consumeValue();
    }
    if (__builtin_expect(!parsed, false)) {
      return false;
    }
    if (__builtin_expect(!parser.match(','), false)) {
      break;
    }
  }
  return parser.match('}');
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
  if (!json_detail::fillStructDirect(obj, parser) || !parser.atEnd()) {
    throw std::runtime_error("invalid JSON");
  }
  return obj;
}

template <typename T>
JsonParseError makeDiagnosticParseError(std::string_view json_str, JsonDiagnostics diagnostics) {
  T obj{};
  json_detail::JsonErrorRecord record;
  json_detail::JsonParser parser(json_str, &record);
  if (json_detail::fillStructDirect(obj, parser)) {
    parser.atEnd();
  }
  if (!record.has) {
    record.record(parser.mark(), "valid JSON");
  }
  return json_detail::buildParseError(json_str, record, diagnostics);
}

template <typename T>
T from_json(std::string_view json_str, JsonDiagnostics diagnostics) {
  T obj{};
  json_detail::JsonParser parser(json_str);
  if (json_detail::fillStructDirect(obj, parser) && parser.atEnd()) {
    return obj;
  }
  throw makeDiagnosticParseError<T>(json_str, diagnostics);
}

} // namespace yuri
