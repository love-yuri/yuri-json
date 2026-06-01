module;

export module yuri.json:parser;

import std;
import :string_utils;
import :output;

// ============================================================================
// JSON 解析器
// ============================================================================

export namespace yuri::json_detail {

/** @brief 解析器：维护当前解析位置 */
struct JsonParser {
  const char *p_;         // 当前解析位置
  const char *const end_; // 输入结束位置

  /** @brief 构造解析器 */
  explicit JsonParser(std::string_view sv) : p_(sv.data()), end_(sv.data() + sv.size()) {
  }

  /** @brief 跳过空白 */
  void skipWs() {
    json_detail::skipWs(p_, end_);
  }

  /** @brief 匹配并消耗一个字符 */
  bool match(char c) {
    skipWs();
    if (p_ < end_ && *p_ == c) {
      ++p_;
      return true;
    }
    return false;
  }

  /** @brief 处理转义字符，成功返回true */
  bool processEscape(std::string &out) {
    if (p_ >= end_) {
      return false;
    }
    char esc = *p_++;
    if (esc == '"') {
      out += '"';
      return true;
    }
    if (esc == '\\') {
      out += '\\';
      return true;
    }
    if (esc == '/') {
      out += '/';
      return true;
    }
    if (esc == 'b') {
      out += '\b';
      return true;
    }
    if (esc == 'f') {
      out += '\f';
      return true;
    }
    if (esc == 'n') {
      out += '\n';
      return true;
    }
    if (esc == 'r') {
      out += '\r';
      return true;
    }
    if (esc == 't') {
      out += '\t';
      return true;
    }
    if (esc == 'u') {
      return processUnicode(out);
    }
    return false;
  }

  /** @brief 处理Unicode转义 \uXXXX */
  bool processUnicode(std::string &out) {
    if (p_ + 4 > end_) {
      return false;
    }
    unsigned int cp = 0;
    for (int i = 0; i < 4; ++i) {
      char h = p_[i];
      cp <<= 4;
      if (h >= '0' && h <= '9') {
        cp |= h - '0';
      } else if (h >= 'a' && h <= 'f') {
        cp |= h - 'a' + 10;
      } else if (h >= 'A' && h <= 'F') {
        cp |= h - 'A' + 10;
      } else {
        return false;
      }
    }
    p_ += 4;
    if (cp < 0x80) {
      out += static_cast<char>(cp);
    } else if (cp < 0x800) {
      out += static_cast<char>(0xC0 | (cp >> 6));
      out += static_cast<char>(0x80 | (cp & 0x3F));
    } else {
      out += static_cast<char>(0xE0 | (cp >> 12));
      out += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
      out += static_cast<char>(0x80 | (cp & 0x3F));
    }
    return true;
  }

  /** @brief 快速解析无转义JSON字符串为string_view并计算FNV-1a哈希 */
  bool parseStringViewHash(std::string_view &out, std::uint32_t &hash) {
    if (!match('"')) {
      return false;
    }
    const char *start = p_;
    hash = 2166136261u;
    while (p_ < end_) {
      char c = *p_;
      if (c == '"') {
        out = std::string_view(start, p_ - start);
        ++p_;
        return true;
      }
      if (c == '\\') {
        return false;
      }
      hash ^= static_cast<std::uint32_t>(c);
      hash *= 16777619u;
      ++p_;
    }
    return false;
  }

  /** @brief 快速解析无转义JSON字符串为string_view */
  bool parseStringView(std::string_view &out) {
    if (!match('"')) {
      return false;
    }
    const char *start = p_;
    const char *seg_end = scanQuoteBackslash(p_, end_);
    if (seg_end >= end_ || *seg_end != '"') {
      return false;
    }
    out = std::string_view(start, seg_end - start);
    p_ = seg_end + 1;
    return true;
  }

  /** @brief SIMD加速解析JSON字符串，结果写入out */
  bool parseDirectString(std::string &out) {
    if (!match('"')) {
      return false;
    }
    out.clear();
    while (p_ < end_) {
      const char *seg_end = scanQuoteBackslash(p_, end_);
      if (seg_end > p_) {
        out.append(p_, seg_end - p_);
      }
      if (seg_end >= end_) {
        p_ = end_;
        return false;
      }
      p_ = seg_end;
      if (*p_ == '"') {
        ++p_;
        return true;
      }
      ++p_;
      if (!processEscape(out)) {
        return false;
      }
    }
    return false;
  }

  /** @brief 解析数值（from_chars高性能） */
  template <typename T>
  bool parseDirectNumber(T &out) {
    skipWs();
    const char *start = p_;
    bool is_float = false;
    if (p_ < end_ && *p_ == '-') {
      ++p_;
    }
    while (p_ < end_ && *p_ >= '0' && *p_ <= '9') {
      ++p_;
    }
    if (p_ < end_ && *p_ == '.') {
      is_float = true;
      ++p_;
      while (p_ < end_ && *p_ >= '0' && *p_ <= '9') {
        ++p_;
      }
    }
    if (p_ < end_ && (*p_ == 'e' || *p_ == 'E')) {
      is_float = true;
      ++p_;
      if (p_ < end_ && (*p_ == '+' || *p_ == '-')) {
        ++p_;
      }
      while (p_ < end_ && *p_ >= '0' && *p_ <= '9') {
        ++p_;
      }
    }
    if (p_ == start) {
      return false;
    }
    std::string_view num_str(start, p_ - start);
    if constexpr (std::is_same_v<T, double> || std::is_same_v<T, float>) {
      bool simple_float = true;
      const char *q = start;
      bool neg = false;
      if (*q == '-') {
        neg = true;
        ++q;
      }
      long long int_part = 0;
      while (q < p_ && *q >= '0' && *q <= '9') {
        int_part = int_part * 10 + (*q - '0');
        ++q;
      }
      double val = static_cast<double>(int_part);
      if (q < p_ && *q == '.') {
        ++q;
        double scale = 0.1;
        while (q < p_ && *q >= '0' && *q <= '9') {
          val += (*q - '0') * scale;
          scale *= 0.1;
          ++q;
        }
      }
      if (q != p_) {
        simple_float = false;
      }
      if (simple_float) {
        out = static_cast<T>(neg ? -val : val);
      } else {
        auto [ptr, ec] = std::from_chars(num_str.data(), num_str.data() + num_str.size(), val);
        if (ec != std::errc{}) {
          return false;
        }
        out = static_cast<T>(val);
      }
    } else {
      const char *q = start;
      bool neg = false;
      if (*q == '-') {
        neg = true;
        ++q;
      }
      long long val = 0;
      while (q < p_) {
        val = val * 10 + (*q - '0');
        ++q;
      }
      out = static_cast<T>(neg ? -val : val);
    }
    return true;
  }

  /** @brief 解析布尔值 */
  bool parseDirectBool(bool &out) {
    skipWs();
    if (p_ + 4 <= end_ && std::memcmp(p_, "true", 4) == 0) {
      p_ += 4;
      out = true;
      return true;
    }
    if (p_ + 5 <= end_ && std::memcmp(p_, "false", 5) == 0) {
      p_ += 5;
      out = false;
      return true;
    }
    return false;
  }

  /** @brief 跳过一个JSON值（类型不匹配时使用） */
  void consumeValue() {
    skipWs();
    if (p_ >= end_) {
      return;
    }
    char c = *p_;
    if (c == '"') {
      ++p_;
      simdSkipString(p_, end_);
      return;
    }
    if (c == '{') {
      consumeObject();
      return;
    }
    if (c == '[') {
      consumeArray();
      return;
    }
    if (c == 't') {
      p_ += 4;
      return;
    }
    if (c == 'f') {
      p_ += 5;
      return;
    }
    if (c == 'n') {
      p_ += 4;
      return;
    }
    while (p_ < end_ && *p_ != ',' && *p_ != '}' && *p_ != ']' && *p_ != ' ' && *p_ != '\t'
           && *p_ != '\n' && *p_ != '\r') {
      ++p_;
    }
  }

  /** @brief 跳过JSON对象 */
  void consumeObject() {
    if (!match('{')) {
      return;
    }
    if (match('}')) {
      return;
    }
    while (true) {
      skipWs();
      if (p_ < end_ && *p_ == '"') {
        ++p_;
        simdSkipString(p_, end_);
      }
      match(':');
      consumeValue();
      if (!match(',')) {
        break;
      }
    }
    match('}');
  }

  /** @brief 跳过JSON数组 */
  void consumeArray() {
    if (!match('[')) {
      return;
    }
    if (match(']')) {
      return;
    }
    while (true) {
      consumeValue();
      if (!match(',')) {
        break;
      }
    }
    match(']');
  }
};

} // namespace yuri::json_detail
