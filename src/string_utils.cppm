module;

export module yuri.json:string_utils;

import std;
import :simd;

// ============================================================================
// SIMD 加速字符串扫描工具
// ============================================================================

export namespace yuri::json_detail {

/** @brief SSE2加速跳过空白字符（16字节并行比较） */
inline void skipWs(const char *&p, const char *end) __attribute__((always_inline));
inline void skipWs(const char *&p, const char *end) {
  // 快速路径：检查第一个字符是否非空白（最常见情况）
  if (__builtin_expect(p < end && *p != ' ' && *p != '\t' && *p != '\n' && *p != '\r', true)) {
    return;
  }
  // 慢速路径：使用SIMD跳过连续空白
  M256i v_sp = simdBroadcast256(0x20);
  M256i v_tab = simdBroadcast256(0x09);
  M256i v_lf = simdBroadcast256(0x0A);
  M256i v_cr = simdBroadcast256(0x0D);
  while (p + 32 <= end) {
    M256i chunk = simdLoad256(p);
    M256i ws = simdOr256(
      simdOr256(simdEq256(chunk, v_sp), simdEq256(chunk, v_tab)),
      simdOr256(simdEq256(chunk, v_lf), simdEq256(chunk, v_cr))
    );
    auto mask = static_cast<unsigned>(simdMovemask256(ws));
    if (mask != 0xFFFFFFFFu) {
      p += __builtin_ctz(~mask);
      return;
    }
    p += 32;
  }
  while (p < end && (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')) {
    ++p;
  }
}

/** @brief SSE2加速扫描引号(0x22)和反斜杠(0x5C)，返回首个匹配位置 */
inline const char *scanQuoteBackslash(const char *p, const char *end)
  __attribute__((always_inline));
inline const char *scanQuoteBackslash(const char *p, const char *end) {
  M256i v_quote = simdBroadcast256(0x22);
  M256i v_bslash = simdBroadcast256(0x5C);
  while (p + 32 <= end) {
    M256i chunk = simdLoad256(p);
    M256i any = simdOr256(simdEq256(chunk, v_quote), simdEq256(chunk, v_bslash));
    int mask = simdMovemask256(any);
    if (mask != 0) {
      return p + __builtin_ctz(mask);
    }
    p += 32;
  }
  while (p < end) {
    if (*p == '"' || *p == '\\') {
      return p;
    }
    ++p;
  }
  return end;
}

/** @brief SSE2加速跳过JSON字符串内容（仅跳过，不解析） */
inline bool simdSkipString(const char *&p, const char *end) __attribute__((always_inline));
inline bool simdSkipString(const char *&p, const char *end) {
  M256i v_quote = simdBroadcast256(0x22);
  M256i v_bslash = simdBroadcast256(0x5C);
  while (p + 32 <= end) {
    M256i chunk = simdLoad256(p);
    M256i any = simdOr256(simdEq256(chunk, v_quote), simdEq256(chunk, v_bslash));
    int mask = simdMovemask256(any);
    if (mask != 0) {
      int pos = __builtin_ctz(mask);
      p += pos;
      if (*p == '\\') {
        if (p + 1 >= end) {
          return false;
        }
        p += 2;
        continue;
      }
      ++p;
      return true;
    }
    p += 32;
  }
  while (p < end) {
    if (*p == '\\') {
      if (p + 1 >= end) {
        return false;
      }
      p += 2;
      continue;
    }
    if (*p == '"') {
      ++p;
      return true;
    }
    ++p;
  }
  return false;
}

} // namespace yuri::json_detail
