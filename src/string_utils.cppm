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
  M128i v_sp = simdBroadcast(0x20);
  M128i v_tab = simdBroadcast(0x09);
  M128i v_lf = simdBroadcast(0x0A);
  M128i v_cr = simdBroadcast(0x0D);
  while (p + 16 <= end) {
    M128i chunk = simdLoad(p);
    M128i ws = simdOr(
      simdOr(simdEq(chunk, v_sp), simdEq(chunk, v_tab)),
      simdOr(simdEq(chunk, v_lf), simdEq(chunk, v_cr))
    );
    int mask = simdMovemask(ws);
    if (mask != 0xFFFF) {
      p += __builtin_ctz(~mask);
      return;
    }
    p += 16;
  }
  while (p < end && (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')) {
    ++p;
  }
}

/** @brief SSE2加速扫描引号(0x22)和反斜杠(0x5C)，返回首个匹配位置 */
inline const char *scanQuoteBackslash(const char *p, const char *end)
  __attribute__((always_inline));
inline const char *scanQuoteBackslash(const char *p, const char *end) {
  M128i v_quote = simdBroadcast(0x22);
  M128i v_bslash = simdBroadcast(0x5C);
  while (p + 16 <= end) {
    M128i chunk = simdLoad(p);
    M128i any = simdOr(simdEq(chunk, v_quote), simdEq(chunk, v_bslash));
    int mask = simdMovemask(any);
    if (mask != 0) {
      return p + __builtin_ctz(mask);
    }
    p += 16;
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
inline void simdSkipString(const char *&p, const char *end) __attribute__((always_inline));
inline void simdSkipString(const char *&p, const char *end) {
  M128i v_quote = simdBroadcast(0x22);
  M128i v_bslash = simdBroadcast(0x5C);
  while (p + 16 <= end) {
    M128i chunk = simdLoad(p);
    M128i any = simdOr(simdEq(chunk, v_quote), simdEq(chunk, v_bslash));
    int mask = simdMovemask(any);
    if (mask != 0) {
      int pos = __builtin_ctz(mask);
      p += pos;
      if (*p == '\\') {
        p += 2;
        continue;
      }
      ++p;
      return;
    }
    p += 16;
  }
  while (p < end) {
    if (*p == '\\') {
      p += 2;
      continue;
    }
    if (*p == '"') {
      ++p;
      return;
    }
    ++p;
  }
}

} // namespace yuri::json_detail
