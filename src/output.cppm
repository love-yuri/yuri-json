module;

#include <cstddef>

export module yuri.json:output;

import std;
import :string_utils;

// ============================================================================
// JSON 输出：数值、字符串、数组序列化
// ============================================================================

export namespace yuri::json_detail {

// 预分配的栈缓冲区大小（64KB，足够容纳大部分JSON输出）
inline constexpr std::size_t kStackBufSize = 65536;

/** @brief 栈缓冲区，避免频繁内存分配 */
struct StackBuffer {
  char data[kStackBufSize];
  char *ptr;

  StackBuffer() : ptr(data) {
  }

  void push(char c) {
    *ptr++ = c;
  }

  void write(const char *s, std::size_t len) {
    std::memcpy(ptr, s, len);
    ptr += len;
  }

  void write(std::string_view sv) {
    write(sv.data(), sv.size());
  }

  std::size_t size() const {
    return ptr - data;
  }

  void flushTo(std::string &out) {
    out.append(data, size());
    ptr = data;
  }
};

struct DirectBuffer {
  char *data;
  char *ptr;

  explicit DirectBuffer(char *out) : data(out), ptr(out) {
  }

  void push(char c) {
    *ptr++ = c;
  }

  void write(const char *s, std::size_t len) {
    std::memcpy(ptr, s, len);
    ptr += len;
  }

  void write(std::string_view sv) {
    write(sv.data(), sv.size());
  }

  std::size_t size() const {
    return ptr - data;
  }
};

inline constexpr char kDigits100[200] = {
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

template <typename Buffer>
[[gnu::always_inline]] inline void writeUIntBuf(Buffer &buf, unsigned long long val) {
  char tmp[20];
  char *p = tmp + sizeof(tmp);
  while (val >= 100) {
    auto idx = static_cast<unsigned>((val % 100) * 2);
    val /= 100;
    *--p = kDigits100[idx + 1];
    *--p = kDigits100[idx];
  }
  if (val < 10) {
    *--p = static_cast<char>('0' + val);
  } else {
    auto idx = static_cast<unsigned>(val * 2);
    *--p = kDigits100[idx + 1];
    *--p = kDigits100[idx];
  }
  buf.write(p, (tmp + sizeof(tmp)) - p);
}

/** @brief 快速整数转字符串（分组查表） */
inline void writeInt(std::string &out, long long val) __attribute__((always_inline));
inline void writeInt(std::string &out, long long val) {
  char tmp[21];
  DirectBuffer buf(tmp);
  if (val < 0) {
    buf.push('-');
    writeUIntBuf(buf, 0ull - static_cast<unsigned long long>(val));
  } else {
    writeUIntBuf(buf, static_cast<unsigned long long>(val));
  }
  out.append(tmp, buf.size());
}

/** @brief 快速整数转字符串（直接使用to_chars，用于大数组） */
inline void writeIntFast(std::string &out, long long val) __attribute__((always_inline));
inline void writeIntFast(std::string &out, long long val) {
  std::array<char, 32> buf{};
  auto [ptr, ec] = std::to_chars(buf.data(), buf.data() + buf.size(), val);
  out.append(buf.data(), ptr - buf.data());
}

/** @brief 写入整数到缓冲区 */
template <typename Buffer>
inline void writeIntBuf(Buffer &buf, long long val) __attribute__((always_inline));
template <typename Buffer>
inline void writeIntBuf(Buffer &buf, long long val) {
  if (val < 0) {
    buf.push('-');
    writeUIntBuf(buf, 0ull - static_cast<unsigned long long>(val));
  } else {
    writeUIntBuf(buf, static_cast<unsigned long long>(val));
  }
}

template <typename Buffer>
inline void writeFloatBuf(Buffer &buf, double val) __attribute__((always_inline));

/** @brief 浮点数转字符串（快速路径 + to_chars回退） */
inline void writeFloat(std::string &out, double val) __attribute__((always_inline));
inline void writeFloat(std::string &out, double val) {
  char tmp[64];
  DirectBuffer buf(tmp);
  writeFloatBuf(buf, val);
  out.append(tmp, buf.size());
}

/** @brief 浮点数转字符串（直接使用to_chars，用于大数组） */
inline void writeFloatFast(std::string &out, double val) __attribute__((always_inline));
inline void writeFloatFast(std::string &out, double val) {
  std::array<char, 64> buf{};
  auto [ptr, ec] = std::to_chars(buf.data(), buf.data() + buf.size(), val);
  out.append(buf.data(), ptr - buf.data());
}

template <typename Buffer>
inline void writeFixedPositiveBuf(Buffer &buf, long long scaled, int digits)
  __attribute__((always_inline));
template <typename Buffer>
inline void writeFixedPositiveBuf(Buffer &buf, long long scaled, int digits) {
  static constexpr int kPow10[] = { 1, 10, 100, 1000 };
  int scale = kPow10[digits];
  writeIntBuf(buf, scaled / scale);
  auto frac = static_cast<unsigned>(scaled % scale);
  if (frac == 0) {
    return;
  }
  while (digits > 1 && frac % 10 == 0) {
    frac /= 10;
    --digits;
  }
  buf.push('.');
  if (digits == 1) {
    buf.push(static_cast<char>('0' + frac));
  } else if (digits == 2) {
    buf.write(&kDigits100[frac * 2], 2);
  } else {
    buf.push(static_cast<char>('0' + frac / 100));
    buf.write(&kDigits100[(frac % 100) * 2], 2);
  }
}

/** @brief 写入浮点数到缓冲区 */
template <typename Buffer>
inline void writeFloatBuf(Buffer &buf, double val) __attribute__((always_inline));
template <typename Buffer>
inline void writeFloatBuf(Buffer &buf, double val) {
  if (__builtin_expect(val >= 0.0 && val < 1000.0, true)) {
    double scaled1 = val * 10.0;
    auto iv1 = static_cast<long long>(scaled1 + 0.5);
    if (__builtin_expect(static_cast<double>(iv1) == scaled1, true)) {
      writeFixedPositiveBuf(buf, iv1, 1);
      return;
    }
    double scaled2 = val * 100.0;
    auto iv2 = static_cast<long long>(scaled2 + 0.5);
    if (__builtin_expect(static_cast<double>(iv2) == scaled2, true)) {
      writeFixedPositiveBuf(buf, iv2, 2);
      return;
    }
    double scaled3 = val * 1000.0;
    auto iv3 = static_cast<long long>(scaled3 + 0.5);
    if (__builtin_expect(static_cast<double>(iv3) == scaled3, true)) {
      writeFixedPositiveBuf(buf, iv3, 3);
      return;
    }
  }
  std::array<char, 64> tmp{};
  auto [ptr, ec] = std::to_chars(tmp.data(), tmp.data() + tmp.size(), val);
  buf.write(tmp.data(), ptr - tmp.data());
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
    const char *found = scanNeedEscape(p, end);
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

/** @brief 写入转义字符串到缓冲区 */
template <typename Buffer>
inline void writeEscapedStringBuf(Buffer &buf, std::string_view sv) __attribute__((always_inline));
template <typename Buffer>
inline void writeEscapedStringBuf(Buffer &buf, std::string_view sv) {
  buf.push('"');
  const char *p = sv.data();
  const char *end = p + sv.size();
  const char *seg = p;
  while (p < end) {
    const char *found = scanNeedEscape(p, end);
    if (found > seg) {
      buf.write(seg, found - seg);
      seg = found;
    }
    if (found >= end) {
      break;
    }
    p = found;
    char c = *p++;
    if (c == '"') {
      buf.write("\\\"", 2);
    } else if (c == '\\') {
      buf.write("\\\\", 2);
    } else if (c == '\n') {
      buf.write("\\n", 2);
    } else if (c == '\r') {
      buf.write("\\r", 2);
    } else if (c == '\t') {
      buf.write("\\t", 2);
    } else {
      static constexpr char kHex[] = "0123456789abcdef";
      auto uc = static_cast<unsigned char>(c);
      char tmp[6] = { '\\', 'u', '0', '0', kHex[(uc >> 4) & 0xF], kHex[uc & 0xF] };
      buf.write(tmp, 6);
    }
    seg = p;
  }
  if (seg < end) {
    buf.write(seg, end - seg);
  }
  buf.push('"');
}

/** @brief 写入已知安全的JSON key */
inline void writeKey(std::string &out, std::string_view key) __attribute__((always_inline));
inline void writeKey(std::string &out, std::string_view key) {
  out += '"';
  out.append(key.data(), key.size());
  out += '"';
}

/** @brief 写入key到缓冲区 */
template <typename Buffer>
inline void writeKeyBuf(Buffer &buf, std::string_view key) __attribute__((always_inline));
template <typename Buffer>
inline void writeKeyBuf(Buffer &buf, std::string_view key) {
  buf.push('"');
  buf.write(key);
  buf.push('"');
}

inline std::size_t escapedStringSize(std::string_view sv) {
  std::size_t size = 2;
  const char *p = sv.data();
  const char *end = p + sv.size();
  const char *seg = p;
  while (p < end) {
    const char *found = scanNeedEscape(p, end);
    size += static_cast<std::size_t>(found - seg);
    if (found >= end) {
      break;
    }
    auto c = static_cast<unsigned char>(*found);
    size += (c == '"' || c == '\\' || c == '\n' || c == '\r' || c == '\t') ? 2 : 6;
    p = found + 1;
    seg = p;
  }
  if (seg < end) {
    size += static_cast<std::size_t>(end - seg);
  }
  return size;
}

inline std::size_t intSize(long long val) {
  auto u =
    val < 0 ? 0ull - static_cast<unsigned long long>(val) : static_cast<unsigned long long>(val);
  std::size_t size = val < 0 ? 1 : 0;
  do {
    ++size;
    u /= 10;
  } while (u != 0);
  return size;
}

inline constexpr std::size_t kFloatSizeUpperBound = 24;

/** @brief 写入字符串数组（使用栈缓冲区优化） */
inline void writeStringArray(std::string &out, const std::vector<std::string> &arr) {
  if (arr.empty()) {
    out += '[';
    out += ']';
    return;
  }
  StackBuffer buf;
  buf.push('[');
  writeEscapedStringBuf(buf, arr[0]);
  for (std::size_t i = 1; i < arr.size(); ++i) {
    buf.push(',');
    // 检查缓冲区是否需要刷新
    if (buf.size() > kStackBufSize - 256) {
      buf.flushTo(out);
    }
    writeEscapedStringBuf(buf, arr[i]);
  }
  buf.push(']');
  buf.flushTo(out);
}

/** @brief 写入整数数组（使用栈缓冲区优化） */
inline void writeIntArray(std::string &out, const std::vector<int> &arr) {
  if (arr.empty()) {
    out += '[';
    out += ']';
    return;
  }
  StackBuffer buf;
  buf.push('[');
  writeIntBuf(buf, arr[0]);
  for (std::size_t i = 1; i < arr.size(); ++i) {
    buf.push(',');
    writeIntBuf(buf, arr[i]);
  }
  buf.push(']');
  buf.flushTo(out);
}

/** @brief 写入浮点数组（使用栈缓冲区优化） */
inline void writeDoubleArray(std::string &out, const std::vector<double> &arr) {
  if (arr.empty()) {
    out += '[';
    out += ']';
    return;
  }
  StackBuffer buf;
  buf.push('[');
  writeFloatBuf(buf, arr[0]);
  for (std::size_t i = 1; i < arr.size(); ++i) {
    buf.push(',');
    writeFloatBuf(buf, arr[i]);
  }
  buf.push(']');
  buf.flushTo(out);
}

} // namespace yuri::json_detail
