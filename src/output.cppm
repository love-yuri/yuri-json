module;

export module yuri.json:output;

import std;
import :string_utils;

// ============================================================================
// JSON 输出：数值、字符串、数组序列化
// ============================================================================

export namespace yuri::json_detail {

/** @brief 快速整数转字符串（0-999查表 + to_chars回退） */
inline void writeInt(std::string &out, long long val) __attribute__((always_inline));
inline void writeInt(std::string &out, long long val) {
  if (val >= 0 && val < 1000) {
    static constexpr char kDigits[200] = {
      '0', '0', '0', '1', '0', '2', '0', '3', '0', '4', '0', '5', '0', '6', '0', '7', '0', '8', '0',
      '9', '1', '0', '1', '1', '1', '2', '1', '3', '1', '4', '1', '5', '1', '6', '1', '7', '1', '8',
      '1', '9', '2', '0', '2', '1', '2', '2', '2', '3', '2', '4', '2', '5', '2', '6', '2', '7', '2',
      '8', '2', '9', '3', '0', '3', '1', '3', '2', '3', '3', '3', '4', '3', '5', '3', '6', '3', '7',
      '3', '8', '3', '9', '4', '0', '4', '1', '4', '2', '4', '3', '4', '4', '4', '5', '4', '6', '4',
      '7', '4', '8', '4', '9', '5', '0', '5', '1', '5', '2', '5', '3', '5', '4', '5', '5', '5', '6',
      '5', '7', '5', '8', '5', '9', '6', '0', '6', '1', '6', '2', '6', '3', '6', '4', '6', '5', '6',
      '6', '6', '7', '6', '8', '6', '9', '7', '0', '7', '1', '7', '2', '7', '3', '7', '4', '7', '5',
      '7', '6', '7', '7', '7', '8', '7', '9', '8', '0', '8', '1', '8', '2', '8', '3', '8', '4', '8',
      '5', '8', '6', '8', '7', '8', '8', '8', '9', '9', '0', '9', '1', '9', '2', '9', '3', '9', '4',
      '9', '5', '9', '6', '9', '7', '9', '8', '9', '9',
    };
    if (val < 10) {
      out += static_cast<char>('0' + val);
    } else if (val < 100) {
      out.append(&kDigits[val * 2], 2);
    } else {
      out += static_cast<char>('0' + val / 100);
      out.append(&kDigits[(val % 100) * 2], 2);
    }
    return;
  }
  if (val < 0) {
    out += '-';
    val = -val;
  }
  std::array<char, 32> buf{};
  auto [ptr, ec] = std::to_chars(buf.data(), buf.data() + buf.size(), val);
  out.append(buf.data(), ptr - buf.data());
}

/** @brief 浮点数转字符串 */
inline void writeFloat(std::string &out, double val) __attribute__((always_inline));
inline void writeFloat(std::string &out, double val) {
  if (__builtin_expect(val >= 0.0 && val < 1000.0, true)) {
    double scaled = val * 10.0;
    auto iv = static_cast<long long>(scaled + 0.5);
    if (__builtin_expect(static_cast<double>(iv) == scaled, true)) {
      writeInt(out, iv / 10);
      if (iv % 10 != 0) {
        out += '.';
        out += static_cast<char>('0' + (iv % 10));
      }
      return;
    }
  }
  std::array<char, 64> buf{};
  auto [ptr, ec] = std::to_chars(buf.data(), buf.data() + buf.size(), val);
  out.append(buf.data(), ptr - buf.data());
}

/** @brief SSE2加速写入转义后的JSON字符串 */
inline void writeEscapedString(std::string &out, std::string_view sv)
  __attribute__((always_inline));
inline void writeEscapedString(std::string &out, std::string_view sv) {
  out += '"';
  const char *p = sv.data();
  const char *end = p + sv.size();
  const char *seg = p;
  while (p < end) {
    const char *found = scanQuoteBackslash(p, end);
    if (found > seg) {
      out.append(seg, found - seg);
      seg = found;
    }
    if (found >= end) {
      break;
    }
    p = found;
    char c = *p++;
    if (c == '"') {
      out += "\\\"";
    } else if (c == '\\') {
      out += "\\\\";
    } else if (c == '\n') {
      out += "\\n";
    } else if (c == '\r') {
      out += "\\r";
    } else if (c == '\t') {
      out += "\\t";
    } else {
      static constexpr char kHex[] = "0123456789abcdef";
      auto uc = static_cast<unsigned char>(c);
      out += "\\u00";
      out += kHex[(uc >> 4) & 0xF];
      out += kHex[uc & 0xF];
    }
    seg = p;
  }
  if (seg < end) {
    out.append(seg, end - seg);
  }
  out += '"';
}

/** @brief 写入已知安全的JSON key */
inline void writeKey(std::string &out, std::string_view key) __attribute__((always_inline));
inline void writeKey(std::string &out, std::string_view key) {
  out += '"';
  out.append(key.data(), key.size());
  out += '"';
}

/** @brief 写入字符串数组 */
inline void writeStringArray(std::string &out, const std::vector<std::string> &arr) {
  out += '[';
  if (!arr.empty()) {
    writeEscapedString(out, arr[0]);
    for (std::size_t i = 1; i < arr.size(); ++i) {
      out += ',';
      writeEscapedString(out, arr[i]);
    }
  }
  out += ']';
}

/** @brief 写入整数数组 */
inline void writeIntArray(std::string &out, const std::vector<int> &arr) {
  out += '[';
  if (!arr.empty()) {
    writeInt(out, arr[0]);
    for (std::size_t i = 1; i < arr.size(); ++i) {
      out += ',';
      writeInt(out, arr[i]);
    }
  }
  out += ']';
}

/** @brief 写入浮点数组 */
inline void writeDoubleArray(std::string &out, const std::vector<double> &arr) {
  out += '[';
  if (!arr.empty()) {
    writeFloat(out, arr[0]);
    for (std::size_t i = 1; i < arr.size(); ++i) {
      out += ',';
      writeFloat(out, arr[i]);
    }
  }
  out += ']';
}

} // namespace yuri::json_detail
