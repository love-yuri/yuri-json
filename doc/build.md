# 构建与测试

## 工具链要求

- GCC 16+（需要 `-freflection`）
- CMake 3.30+
- Ninja
- x86-64 平台

## 构建 demo

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -Wno-dev
cmake --build build
./build/json_demo
```

## 运行 benchmark

安装可选依赖（Arch Linux）：

```bash
sudo pacman -S nlohmann-json rapidjson glaze simdjson
```

运行：

```bash
bash script/benchmark.sh
```

## 运行 CTest

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON -Wno-dev
cmake --build build
cd build && ctest --output-on-failure
```
