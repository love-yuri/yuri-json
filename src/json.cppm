export module yuri.json;

import :simd;
import :string_utils;
import :output;
import :parser;
export import :reflection;

// ============================================================================
// yuri::json — 基于C++26静态反射的高性能JSON库
//
// 公开接口：to_json / from_json / JsonParseError / JsonDiagnostics
// 内部分区：simd / string_utils / output / parser / reflection
// ============================================================================
