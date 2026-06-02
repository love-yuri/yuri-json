module;

export module yuri.json:simd;

import std;

// ============================================================================
// x64 SSE2 SIMD 基础封装
// ============================================================================

export namespace yuri::json_detail {

/** @brief 128位字节向量类型 */
using M128i = char __attribute__((vector_size(16)));

/** @brief 256位字节向量类型 */
using M256i = char __attribute__((vector_size(32)));

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

/** @brief AVX2广播单字节到32字节向量 */
inline M256i simdBroadcast256(unsigned char c) __attribute__((always_inline));
inline M256i simdBroadcast256(unsigned char c) {
  M256i v;
  __builtin_memset(&v, static_cast<int>(c), 32);
  return v;
}

/** @brief AVX2未对齐加载32字节 */
inline M256i simdLoad256(const char *p) __attribute__((always_inline));
inline M256i simdLoad256(const char *p) {
  M256i v;
  __builtin_memcpy(&v, p, 32);
  return v;
}

/** @brief AVX2字节相等比较 */
inline M256i simdEq256(M256i a, M256i b) __attribute__((always_inline));
inline M256i simdEq256(M256i a, M256i b) {
  return a == b;
}

/** @brief AVX2按位或 */
inline M256i simdOr256(M256i a, M256i b) __attribute__((always_inline));
inline M256i simdOr256(M256i a, M256i b) {
  return a | b;
}

/** @brief AVX2提取高位掩码 */
inline int simdMovemask256(M256i v) __attribute__((always_inline));
inline int simdMovemask256(M256i v) {
  return __builtin_ia32_pmovmskb256(v);
}

} // namespace yuri::json_detail
