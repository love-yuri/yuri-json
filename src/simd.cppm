module;

export module yuri.json:simd;

import std;

// ============================================================================
// x64 SSE2 SIMD 基础封装
// ============================================================================

export namespace yuri::json_detail {

/** @brief 128位字节向量类型 */
using M128i = char __attribute__((vector_size(16)));

/** @brief SSE2广播单字节到16字节向量 */
inline M128i simdBroadcast(unsigned char c) __attribute__((always_inline));
inline M128i simdBroadcast(unsigned char c) {
  M128i v;
  __builtin_memset(&v, static_cast<int>(c), 16);
  return v;
}

/** @brief SSE2未对齐加载16字节 */
inline M128i simdLoad(const char *p) __attribute__((always_inline));
inline M128i simdLoad(const char *p) {
  M128i v;
  __builtin_memcpy(&v, p, 16);
  return v;
}

/** @brief SSE2字节相等比较 */
inline M128i simdEq(M128i a, M128i b) __attribute__((always_inline));
inline M128i simdEq(M128i a, M128i b) {
  return a == b;
}

/** @brief SSE2按位或 */
inline M128i simdOr(M128i a, M128i b) __attribute__((always_inline));
inline M128i simdOr(M128i a, M128i b) {
  return a | b;
}

/** @brief SSE2提取高位掩码 */
inline int simdMovemask(M128i v) __attribute__((always_inline));
inline int simdMovemask(M128i v) {
  return __builtin_ia32_pmovmskb128(v);
}

} // namespace yuri::json_detail
