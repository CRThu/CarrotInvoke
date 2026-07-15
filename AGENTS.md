# CarrotInvoke - 项目架构指南

## 项目概述

CarrotInvoke 是一个轻量级的 C 动态函数调用框架，用于通过字符串命令或参数池调用预注册的函数。常用于嵌入式 RPC 场景。

## 目录结构

```
├── inc/                 # 公开头文件
│   ├── dynpool.h       # 动态类型池 (类型定义 + 参数存储)
│   ├── cmdparse.h      # 命令字符串解析
│   └── dyncall.h       # 动态调用引擎
├── src/                 # 实现
│   ├── dynpool.c
│   ├── cmdparse.c
│   └── dyncall.c
├── examples/            # 示例代码
├── tests/               # 单元测试 (使用 Unity + FFF)
├── vendor/              # 第三方库 (unity, fff)
├── build/               # CMake 构建输出
└── CMakeLists.txt       # CMake 配置
```

## 核心模块

### 1. dynpool (动态类型池)
- **职责**: 类型安全的参数存储，支持动态类型系统
- **关键结构**:
  - `dynpool_t`: 参数池，含 buf (1024 bytes)、elements[10]、cursor
  - `dyn_info_t`: 元素元信息 (offset, len, type)
- **类型系统**: T_VOID, T_NULL, T_DEC64, T_HEX64, T_STRING, T_BYTES, T_KV, T_JSON
- **核心 API**:
  - `dynpool_init()` / `dynpool_set()` / `dynpool_get()` / `dynpool_peek()`

### 2. cmdparse (命令解析)
- **职责**: 解析字符串命令格式为 dynpool
- **支持格式**: `func_name(arg1, arg2, ...)` 逗号分隔
- **核心 API**:
  - `cmdparse_from_string(pool, str, &len)` - 从字符串解析
  - `cmdparse_from_buffer(pool, buf, offset, size, &len)` - 从缓冲区解析

### 3. dyncall (动态调用)
- **职责**: 函数注册、查找、动态调用
- **关键结构**:
  - `function_info_t`: 函数信息 (name, handler, ret_type, args_type[], args_count)
  - `function_group_t`: 函数组 (name, func_table[], func_count)
- **宏定义**:
  - `FUNCTION_GROUP("name", ...)` - 定义函数组
  - `FUNCTION_INFO(handler, ret, ...)` - 定义函数信息
- **核心 API**:
  - `register_rpc_group()` - 注册函数组
  - `invoke(cmd, ...)` - 直接调用
  - `invoke_by_cmd(pool)` - 通过解析后的命令调用
  - `invoke_by_pool(pool, func)` - 通过参数池调用
  - `find_func(name)` - 查找函数

## 调用流程

```
字符串命令 → cmdparse → dynpool → dyncall → 函数执行
```

1. 用户提供字符串: `"func_name(arg1, arg2)"`
2. `cmdparse_from_string()` 解析为 `dynpool_t`
3. `invoke_by_cmd()` 查找并调用目标函数

## 限制

- 最多 256 个注册函数
- 单个函数最多 9 个参数
- 参数池 1024 字节
- 最多 8 个函数组

## 构建

```bash
cmake -B build
cmake --build build
```

## 测试

测试文件: `tests/test_*.c`，使用 Unity 框架，构建后运行 `carrot_tests`。
