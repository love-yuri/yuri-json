---
description: 根据项目代码规范格式化指定文件
argument-hint: <file path>
allowed-tools: Read, Edit, Write, Glob, Grep, Bash
---

## 你的任务

你是一个代码格式化助手。根据 `$ARGUMENTS` 指定的文件路径，读取文件并严格按照以下代码规范重新格式化。

## 代码规范

### 命名规范

1. **函数**: 小驼峰 (`addTask`, `getPlayerBar`)
2. **类型**: 大驼峰 (`ThreadPool`, `PlayerBar`, `SongInfo`)
3. **变量**: 下划线命名 (`player_bar`, `cover_image`)
   - 类私有变量且仅有一个单词时添加 `_` 后缀 (`name_`, `bg_`, `alpha_`)
   - 两个及以上单词不加后缀 (`player_bar`, `cover_bg`, `text_alpha`)
4. **常量**: `constexpr` 优先于 `const`，命名用 `k` 前缀大驼峰 (`kBarHeight`, `kCoverSize`)

### 注释规范

1. **所有函数**必须添加完整的 `Doxygen` 注释（`/** ... */`），不同函数之间间隔一行
2. 成员函数仅需在**声明**处添加注释，实现处不重复
3. 全局/独立函数在**前向声明**处添加注释，实现处不重复
4. 代码关键实现需要添加行内注释
5. **所有变量**（类成员、全局变量、全局常量）右侧必须添加中文注释并列对齐

### 函数规范

1. 成员函数实现时该添加 `const` 的必须添加（`const` 成员函数、`const` 参数）
2. 函数定义的值参数如果不修改也要加 `const`
3. 不访问类资源的成员函数应声明为全局函数
   - 前向声明放在文件顶部（带 Doxygen 注释）
   - 实现放在 namespace 外部
4. 类成员函数的声明和实现严格分离（除非特别短，如仅一行的内联 getter）

### 代码风格

1. 参数传递：合理规划，小类型按值传递，大对象按 const 引用传递
2. **代码尽量一行写完**，不要出现以下情况：
   ```cpp
   // BAD - 参数拆行但不够美观
   void PlayerBar::drawVolumeBar(SkCanvas *canvas, const float x, const float cy,
     const float w) const {
   ```
   除非多行参数排列非常整齐好看（如多个长参数对齐排列）
3. 变量声明尽量一行写完

## 执行流程

1. 读取 `$ARGUMENTS` 指定的文件
2. 逐项检查是否符合上述规范
3. 列出需要修改的项目
4. 执行修改
5. 最终验证所有规范已满足
