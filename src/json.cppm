export module yuri.json;

export import :simd;
export import :string_utils;
export import :output;
export import :parser;
export import :reflection;

// ============================================================================
// yuri::json — 基于C++26静态反射的高性能JSON库
//
// 模块分区：
//   :simd           — SSE2 SIMD 基础封装
//   :string_utils   — SIMD 加速字符串扫描
//   :output         — 数值/字符串/数组序列化
//   :parser         — JSON 解析器
//   :reflection     — 反射序列化/反序列化（导出 to_json / from_json）
// ============================================================================
