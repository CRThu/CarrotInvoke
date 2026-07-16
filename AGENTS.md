# CarrotInvoke - 项目架构指南

## 项目概述

CarrotInvoke 是一个轻量级的 C 动态函数调用框架，用于通过字符串命令或参数池调用预注册的函数。常用于嵌入式 RPC 场景。

## 项目规则

### 代码编辑规则
- **禁止使用 `write` 工具修改已有文件**：必须使用 `edit` 工具进行精确替换，防止注释和格式丢失
- `write` 仅用于创建新文件

### 重构规则
- **每次代码重构后必须更新本文档**（AGENTS.md）和相关文档
- 更新内容包括：目录结构、模块说明、API 列表、使用流程示例

## 目录结构

```
├── inc/                 # 公开头文件
│   ├── dynpool.h       # 动态类型池 (类型定义 + 参数存储)
│   ├── cmdparse.h      # 命令字符串解析 (带内存复制)
│   ├── cmdscan.h       # 零拷贝状态机解析器 + 预解析
│   ├── cmdqueue.h      # 命令队列 (环形缓冲区)
│   ├── ringbuf.h       # 通用环形缓冲区 (支持 DMA 硬件同步)
│   └── dyncall.h       # 动态调用引擎
├── src/                 # 实现
│   ├── dynpool.c
│   ├── cmdparse.c
│   ├── cmdscan.c
│   ├── cmdqueue.c
│   ├── ringbuf.c
│   └── dyncall.c
├── examples/            # 示例代码
├── tests/               # 单元测试 (使用 Unity + FFF)
├── vendor/              # 第三方库 (unity, fff)
├── docs/                # 文档
│   └── rpc_usage.md     # 使用指南 (STM32 HAL 风格)
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

### 2. cmdparse (命令解析) - 已被 cmdscan 替代
- **职责**: 解析字符串命令格式为 dynpool
- **支持格式**: `func_name(arg1, arg2, ...)` 逗号分隔
- **核心 API**:
  - `cmdparse_from_string(pool, str, &len)` - 从字符串解析
  - `cmdparse_from_buffer(pool, buf, offset, size, &len)` - 从缓冲区解析

### 3. cmdscan (零拷贝状态机解析器 + 预解析)
- **职责**: 零拷贝命令扫描，支持 DMA 增量接收，预解析函数名长度
- **设计目标**:
  - 无内存复制 - 直接返回原始缓冲区指针
  - 状态机按 byte 扫描 - 适配 DMA 增量接收
  - 无 std 库依赖 - 纯嵌入式友好
  - 预解析函数名长度 - 供 cmd_queue 直接使用
- **关键结构**:
  - `cmd_scanner_t`: 扫描器上下文，维护扫描状态
  - `cmd_prefetch_t`: 预解析结果，含 buf 指针、buf_len、cmd_start、cmd_len、func_len
  - `cmd_parse_result_t`: 解析结果，包含函数名指针和参数指针数组
  - `cmd_arg_t`: 参数指针 (ptr + len)
- **核心 API**:
  - `cmdscan_init(scanner, buf, size)` - 初始化扫描器
  - `cmdscan_scan(scanner)` - 状态机扫描命令（支持增量接收）
  - `cmdscan_prefetch(scanner, &prefetch)` - 预解析单条命令（记录扫描位置）
  - `cmdparse_args(cmd, len, result)` - 解析为参数指针数组（零拷贝，执行时使用）
  - `cmdscan_next(scanner, next_start)` - 获取下一个命令位置
- **使用流程（状态机）**:
  ```c
  cmd_scanner_t scanner;
  cmdscan_init(&scanner, dma_buf, dma_len);
  
  while (1) {
      scan_status_t status = cmdscan_scan(&scanner);
      if (status == SCAN_COMPLETE) {
          cmd_parse_result_t result;
          cmdparse_args(scanner.buf + scanner.cmd_start, 
                        scanner.cmd_len, &result);
      }
  }
  ```
- **使用流程（预解析 + 队列）**:
  ```c
  cmd_scanner_t scanner;
  cmdscan_init(&scanner, dma_buf, dma_len);
  
  cmd_queue_t queue;
  cmd_queue_init(&queue);
  
  // 循环预解析所有命令，scanner 自动记录位置
  cmd_prefetch_t prefetch;
  while (cmdscan_prefetch(&scanner, &prefetch) == SCAN_COMPLETE) {
      cmd_queue_push(&queue, &prefetch);
  }
  
  // 出队时再解析
  cmd_prefetch_t pf;
  while (cmd_queue_pop(&queue, &pf)) {
      cmdparse_args(pf.buf + pf.cmd_start, pf.cmd_len, &result);
  }
  ```

### 4. cmdqueue (命令队列)
- **职责**: 命令排队，支持协作式中断检查
- **设计目标**:
  - 内部 buf 存储原始命令（独立于 DMA 缓冲区）
  - 支持环形/非环形 DMA 缓冲区
- **关键结构**:
  - `cmd_queue_t`: 命令队列（内部 items[] 使用 `cmd_prefetch_t`）
- **核心 API**:
  - `cmd_queue_init(queue)` - 初始化队列
  - `cmd_queue_push(queue, &prefetch)` - 入队（从 prefetch 复制）
  - `cmd_queue_pop(queue, &prefetch)` - 出队（返回 cmd_prefetch_t）
  - `cmd_queue_check(queue, func_name)` - 扫描指定函数名
  - `cmd_queue_flush(queue)` - 清空队列

### 5. ringbuf (通用环形缓冲区)
- **职责**: 通用环形缓冲区，支持 DMA 接收/发送、命令队列等场景
- **设计目标**:
  - 支持读写操作，wrap-aware
  - 可选硬件同步：DMA 接收(读 head) / DMA 发送(读 tail)
  - 适用于嵌入式 MCU (STM32 / ESP32 等)
- **编译宏**:
  - `RINGBUF_DMA` - 在 `ringbuf.h` 中取消注释启用，开启后支持 DMA 硬件同步（head_reader / tail_reader），关闭后退化为纯软件环形缓冲区
- **关键结构**:
  - `ringbuf_t`: 环形缓冲区管理结构
- **核心 API**:
  - `ringbuf_init(ring, buf, size)` - 初始化
  - `ringbuf_set_head_reader(ring, func)` - 设置 DMA RX 硬件位置读取函数 (需 `RINGBUF_DMA`)
  - `ringbuf_set_tail_reader(ring, func)` - 设置 DMA TX 硬件位置读取函数 (需 `RINGBUF_DMA`)
  - `ringbuf_sync_head(ring)` / `ringbuf_sync_tail(ring)` - 从硬件同步位置 (需 `RINGBUF_DMA`)
  - `ringbuf_set_head(ring, head)` / `ringbuf_set_tail(ring, tail)` - 手动设置
  - `ringbuf_readable(ring)` / `ringbuf_writable(ring)` - 查询空间
  - `ringbuf_write(ring, src, len)` - 写入（处理 wrap-around）
  - `ringbuf_peek(ring, dst, len)` - 读取（不消费）
  - `ringbuf_skip(ring, len)` - 跳过已处理数据
  - `ringbuf_flush(ring)` - 清空缓冲区

### 6. dyncall (动态调用)
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

### 流程 1: 传统方式 (带内存复制)
```
字符串命令 → cmdparse → dynpool → dyncall → 函数执行
```

1. 用户提供字符串: `"func_name(arg1, arg2)"`
2. `cmdparse_from_string()` 解析为 `dynpool_t`
3. `invoke_by_cmd()` 查找并调用目标函数

### 流程 2: 零拷贝方式 (推荐用于嵌入式)
```
DMA 缓冲区 → cmdscan 状态机 → 参数指针数组 → dyncall → 函数执行
```

1. DMA 接收数据到缓冲区
2. `cmdscan_scan()` 扫描找到完整命令
3. `cmdparse_args()` 解析为参数指针数组（零拷贝）
4. 使用 `invoke_by_pool()` 或直接调用函数

### 流程 3: 预解析 + 队列方式 (推荐用于需要排队的场景)
```
DMA 缓冲区 → cmdscan_prefetch → cmd_queue → 出队 → cmdparse_args → 函数执行
```

1. DMA 接收数据到缓冲区
2. `cmdscan_prefetch()` 预解析命令边界和函数名长度
3. `cmd_queue_push()` 复制命令到队列内部缓冲区
4. `cmd_queue_pop()` 出队获取 cmd_prefetch_t
5. `cmdparse_args(pf.buf + pf.cmd_start, pf.cmd_len, &result)` 执行时解析参数

## 限制

- 最多 256 个注册函数
- 单个函数最多 9 个参数
- 参数池 1024 字节
- 最多 8 个函数组
- 队列最多 128 条命令
- 队列内部缓冲区 2048 字节 (环形)
- DMA 环形缓冲区大小由用户定义

## 构建

**必须使用 `build.bat` 构建**，不要直接调用 cmake。build.bat 自动检测编译器 (GCC/MSVC) 并使用 Ninja。

```bat
build.bat          # 构建
build.bat run      # 构建并运行测试
build.bat demo     # 构建并运行 demo
```

## 测试

测试文件: `tests/test_*.c`，使用 Unity 框架，构建后运行 `carrot_tests`。

## 文档

详细使用指南请参考: [docs/rpc_usage.md](docs/rpc_usage.md)

包含：
- API 参考
- 5 种使用场景（同步/异步/中断/混合）
- DMA 集成指南（STM32 HAL / ESP32 / 轮询）
- 移植指南
- 常见问题
