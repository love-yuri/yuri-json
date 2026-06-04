module;

export module yuri.json:simd;

import std;

// ============================================================================
// x64 SSE2 SIMD 基础封装
// ============================================================================

export namespace yuri::json_detail {

/** @brief 128位字节向量类型 */
using M128i = char __attribute__((vector_size(16)));

/** @brief 由两个128位向量组成的32字节块 */
struct M256i {
  M128i lo; // 低128位
  M128i hi; // 高128位
};

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

/** @brief 广播单字节到32字节块 */
inline M256i simdBroadcast256(unsigned char c) __attribute__((always_inline));
inline M256i simdBroadcast256(unsigned char c) {
  M128i v = simdBroadcast(c);
  return { v, v };
}

/** @brief 未对齐加载32字节 */
inline M256i simdLoad256(const char *p) __attribute__((always_inline));
inline M256i simdLoad256(const char *p) {
  return { simdLoad(p), simdLoad(p + 16) };
}

/** @brief 32字节相等比较 */
inline M256i simdEq256(M256i a, M256i b) __attribute__((always_inline));
inline M256i simdEq256(M256i a, M256i b) {
  return { simdEq(a.lo, b.lo), simdEq(a.hi, b.hi) };
}

/** @brief 32字节按位或 */
inline M256i simdOr256(M256i a, M256i b) __attribute__((always_inline));
inline M256i simdOr256(M256i a, M256i b) {
  return { simdOr(a.lo, b.lo), simdOr(a.hi, b.hi) };
}

/** @brief 提取32字节高位掩码 */
inline int simdMovemask256(M256i v) __attribute__((always_inline));
inline int simdMovemask256(M256i v) {
  return simdMovemask(v.lo) | (simdMovemask(v.hi) << 16);
}

/** @brief SSE2无符号字节小于比较（a < b）：高7位全零且低7位 < b */
inline M128i simdUnsignedLt(M128i a, M128i b) __attribute__((always_inline));
inline M128i simdUnsignedLt(M128i a, M128i b) {
  M128i high_bit = simdBroadcast(0x80);
  M128i no_high = a & ~high_bit; // 清除符号位，保留低7位
  // 有符号比较：低7位 < b 且 高位无设置（即 a < 0x80）
  return __builtin_ia32_pcmpgtb128(b, no_high) & ~__builtin_ia32_pcmpgtb128(a, high_bit);
}

/** @brief 32字节无符号字节小于比较 */
inline M256i simdUnsignedLt256(M256i a, M256i b) __attribute__((always_inline));
inline M256i simdUnsignedLt256(M256i a, M256i b) {
  return { simdUnsignedLt(a.lo, b.lo), simdUnsignedLt(a.hi, b.hi) };
}

} // namespace yuri::json_detail
