module;

export module yuri.json:parser;

import std;
import :string_utils;
import :output;

// ============================================================================
// JSON 解析器（极致优化版 v2）
// ============================================================================

export namespace yuri::json_detail {

/** @brief 解析器：维护当前解析位置 */
struct JsonParser {
  const char *p_;         // 当前解析位置
  const char *const end_; // 输入结束位置

  /** @brief 构造解析器 */
  explicit JsonParser(std::string_view sv) : p_(sv.data()), end_(sv.data() + sv.size()) {
  }

  const char *mark() const {
    return p_;
  }

  void rewind(const char *p) {
    p_ = p;
  }

  bool nextIsWs() const {
    return p_ < end_ && (*p_ == ' ' || *p_ == '\t' || *p_ == '\n' || *p_ == '\r');
  }

  /** @brief 跳过空白 */
  void skipWs() {
    json_detail::skipWs(p_, end_);
  }

  /** @brief 匹配并消耗一个字符（跳过前导空白） */
  bool match(char c) {
    skipWs();
    if (__builtin_expect(p_ < end_ && *p_ == c, true)) {
      ++p_;
      return true;
    }
    return false;
  }

  /** @brief 匹配字符（不跳过空白，用于已知无空白的场景） */
  bool matchNoWs(char c) {
    if (__builtin_expect(p_ < end_ && *p_ == c, true)) {
      ++p_;
      return true;
    }
    return false;
  }

  bool matchNoWsOrWs(char c) {
    if (__builtin_expect(p_ < end_ && *p_ == c, true)) {
      ++p_;
      return true;
    }
    if (__builtin_expect(nextIsWs(), false)) {
      return match(c);
    }
    return false;
  }

  bool matchKeyNoWs(std::string_view key) {
    auto len = key.size();
    if (
      __builtin_expect(
        p_ + len + 2 <= end_ && p_[0] == '"' && p_[len + 1] == '"'
          && std::memcmp(p_ + 1, key.data(), len) == 0,
        true
      )
    ) {
      p_ += len + 2;
      return true;
    }
    return false;
  }

  /** @brief 处理转义字符，成功返回true */
  bool processEscape(std::string &out) {
    if (__builtin_expect(p_ >= end_, false)) {
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
    if (__builtin_expect(p_ + 4 > end_, false)) {
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
    if (__builtin_expect(!match('"'), false)) {
      return false;
    }
    const char *start = p_;
    hash = 2166136261u;
    while (__builtin_expect(p_ < end_, true)) {
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

  /** @brief SIMD加速解析JSON字符串，结果写入out */
  bool parseDirectString(std::string &out) {
    if (__builtin_expect(!match('"'), false)) {
      return false;
    }
    out.clear();
    while (__builtin_expect(p_ < end_, true)) {
      const char *seg_end = scanQuoteBackslash(p_, end_);
      if (seg_end > p_) {
        out.append(p_, seg_end - p_);
      }
      if (__builtin_expect(seg_end >= end_, false)) {
        p_ = end_;
        return false;
      }
      p_ = seg_end;
      if (*p_ == '"') {
        ++p_;
        return true;
      }
      ++p_;
      if (__builtin_expect(!processEscape(out), false)) {
        return false;
      }
    }
    return false;
  }

  /** @brief 解析整数（带前导空白跳过） */
  template <typename T>
    requires std::is_integral_v<T>
  bool parseDirectNumber(T &out) {
    skipWs();
    return parseIntNoWs(out);
  }

  /** @brief 解析整数（无前导空白跳过，用于数组批量解析） */
  template <typename T>
    requires std::is_integral_v<T>
  bool parseIntNoWs(T &out) {
    const char *start = p_;
    bool neg = false;
    if (__builtin_expect(p_ < end_ && *p_ == '-', false)) {
      neg = true;
      ++p_;
    }
    if (__builtin_expect(p_ >= end_ || *p_ < '0' || *p_ > '9', false)) {
      p_ = start;
      return false;
    }
    // 快速路径：小整数（最多10位）
    if (__builtin_expect(p_ + 10 <= end_, true)) {
      long long val = 0;
      const char *q = p_;
      // 手动展开循环
      if (*q >= '0' && *q <= '9') {
        val = val * 10 + (*q++ - '0');
      }
      if (*q >= '0' && *q <= '9') {
        val = val * 10 + (*q++ - '0');
      }
      if (*q >= '0' && *q <= '9') {
        val = val * 10 + (*q++ - '0');
      }
      if (*q >= '0' && *q <= '9') {
        val = val * 10 + (*q++ - '0');
      }
      if (*q >= '0' && *q <= '9') {
        val = val * 10 + (*q++ - '0');
      }
      if (*q >= '0' && *q <= '9') {
        val = val * 10 + (*q++ - '0');
      }
      if (*q >= '0' && *q <= '9') {
        val = val * 10 + (*q++ - '0');
      }
      if (*q >= '0' && *q <= '9') {
        val = val * 10 + (*q++ - '0');
      }
      if (*q >= '0' && *q <= '9') {
        val = val * 10 + (*q++ - '0');
      }
      if (*q >= '0' && *q <= '9') {
        val = val * 10 + (*q++ - '0');
      }
      if (q < end_ && *q >= '0' && *q <= '9') {
        while (p_ < end_ && *p_ >= '0' && *p_ <= '9') {
          ++p_;
        }
        std::string_view num_str(start + (neg ? 1 : 0), p_ - start - (neg ? 1 : 0));
        auto [ptr, ec] = std::from_chars(num_str.data(), num_str.data() + num_str.size(), out);
        if (ec != std::errc{}) {
          return false;
        }
        if (neg) {
          out = -out;
        }
        return true;
      }
      p_ = q;
      out = static_cast<T>(neg ? -val : val);
      return true;
    }
    while (p_ < end_ && *p_ >= '0' && *p_ <= '9') {
      ++p_;
    }
    std::string_view num_str(start + (neg ? 1 : 0), p_ - start - (neg ? 1 : 0));
    auto [ptr, ec] = std::from_chars(num_str.data(), num_str.data() + num_str.size(), out);
    if (ec != std::errc{}) {
      return false;
    }
    if (neg) {
      out = -out;
    }
    return true;
  }

  /** @brief 解析浮点数（带前导空白跳过） */
  template <typename T>
    requires std::is_floating_point_v<T>
  bool parseDirectNumber(T &out) {
    skipWs();
    return parseFloatNoWs(out);
  }

  /** @brief 解析浮点数（无前导空白跳过，用于数组批量解析） */
  template <typename T>
    requires std::is_floating_point_v<T>
  bool parseFloatNoWs(T &out) {
    const char *start = p_;
    bool neg = false;
    if (__builtin_expect(p_ < end_ && *p_ == '-', false)) {
      neg = true;
      ++p_;
    }
    if (__builtin_expect(p_ >= end_ || *p_ < '0' || *p_ > '9', false)) {
      p_ = start;
      return false;
    }
    // 快速路径：简单浮点数（无指数）
    long long int_part = 0;
    int digit_count = 0;
    while (p_ < end_ && *p_ >= '0' && *p_ <= '9' && digit_count < 10) {
      int_part = int_part * 10 + (*p_ - '0');
      ++p_;
      ++digit_count;
    }
    while (p_ < end_ && *p_ >= '0' && *p_ <= '9') {
      ++p_;
    }
    double val = static_cast<double>(int_part);
    bool is_simple = true;
    if (__builtin_expect(p_ < end_ && *p_ == '.', true)) {
      ++p_;
      long long frac_part = 0;
      int frac_digits = 0;
      while (p_ < end_ && *p_ >= '0' && *p_ <= '9' && frac_digits < 10) {
        frac_part = frac_part * 10 + (*p_ - '0');
        ++p_;
        ++frac_digits;
      }
      while (p_ < end_ && *p_ >= '0' && *p_ <= '9') {
        ++p_;
      }
      if (__builtin_expect(frac_digits <= 10, true)) {
        static constexpr double kPow10[] = { 1.0,        0.1,         0.01,        0.001,
                                             0.0001,     0.00001,     0.000001,    0.0000001,
                                             0.00000001, 0.000000001, 0.0000000001 };
        val += static_cast<double>(frac_part) * kPow10[frac_digits];
      } else {
        is_simple = false;
      }
    } else {
      is_simple = (digit_count <= 10);
    }
    if (p_ < end_ && (*p_ == 'e' || *p_ == 'E')) {
      is_simple = false;
      ++p_;
      if (p_ < end_ && (*p_ == '+' || *p_ == '-')) {
        ++p_;
      }
      while (p_ < end_ && *p_ >= '0' && *p_ <= '9') {
        ++p_;
      }
    }
    if (__builtin_expect(is_simple, true)) {
      out = static_cast<T>(neg ? -val : val);
    } else {
      std::string_view num_str(start, p_ - start);
      double tmp;
      auto [ptr, ec] = std::from_chars(num_str.data(), num_str.data() + num_str.size(), tmp);
      if (ec != std::errc{}) {
        return false;
      }
      out = static_cast<T>(tmp);
    }
    return true;
  }

  /** @brief 解析布尔值 */
  bool parseDirectBool(bool &out) {
    skipWs();
    if (__builtin_expect(p_ + 4 <= end_ && std::memcmp(p_, "true", 4) == 0, true)) {
      p_ += 4;
      out = true;
      return true;
    }
    if (__builtin_expect(p_ + 5 <= end_ && std::memcmp(p_, "false", 5) == 0, true)) {
      p_ += 5;
      out = false;
      return true;
    }
    return false;
  }

  void reserveNumericArray(std::vector<int> &arr) {
    auto remaining = static_cast<std::size_t>(end_ - p_);
    if (remaining > 4096) {
      arr.reserve(512);
    } else if (remaining > 256) {
      arr.reserve(64);
    } else {
      arr.reserve(8);
    }
  }

  void reserveNumericArray(std::vector<double> &arr) {
    auto remaining = static_cast<std::size_t>(end_ - p_);
    if (remaining > 4096) {
      arr.reserve(512);
    } else if (remaining > 256) {
      arr.reserve(64);
    } else {
      arr.reserve(8);
    }
  }

  /** @brief 批量解析整数数组（极致优化版，减少空白跳过） */
  bool parseIntArray(std::vector<int> &arr) {
    if (__builtin_expect(!match('['), false)) {
      return false;
    }
    arr.clear();
    reserveNumericArray(arr);
    if (__builtin_expect(match(']'), false)) {
      return true;
    }
    while (true) {
      int elem = 0;
      if (__builtin_expect(parseIntNoWs(elem), true)) {
        arr.push_back(elem);
      } else {
        return false;
      }
      if (__builtin_expect(p_ < end_ && *p_ == ',', true)) {
        ++p_;
        if (__builtin_expect(nextIsWs(), false)) {
          skipWs();
        }
        continue;
      }
      if (__builtin_expect(nextIsWs(), false)) {
        skipWs();
      }
      break;
    }
    return matchNoWs(']');
  }

  /** @brief 批量解析浮点数组（极致优化版，减少空白跳过） */
  bool parseDoubleArray(std::vector<double> &arr) {
    if (__builtin_expect(!match('['), false)) {
      return false;
    }
    arr.clear();
    reserveNumericArray(arr);
    if (__builtin_expect(match(']'), false)) {
      return true;
    }
    while (true) {
      double elem = 0.0;
      if (__builtin_expect(parseFloatNoWs(elem), true)) {
        arr.push_back(elem);
      } else {
        return false;
      }
      if (__builtin_expect(p_ < end_ && *p_ == ',', true)) {
        ++p_;
        if (__builtin_expect(nextIsWs(), false)) {
          skipWs();
        }
        continue;
      }
      if (__builtin_expect(nextIsWs(), false)) {
        skipWs();
      }
      break;
    }
    return matchNoWs(']');
  }

  /** @brief 跳过一个JSON值（类型不匹配时使用） */
  void consumeValue() {
    skipWs();
    if (__builtin_expect(p_ >= end_, false)) {
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
