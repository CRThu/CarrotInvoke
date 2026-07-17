## CarrotInvoke - 动态函数调用框架

🌐 Language: [English](README.md) | [中文](README.zh-CN.md)

---

![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)

CarrotInvoke 是一个轻量级的 C 动态函数调用框架，适用于嵌入式 RPC 场景。通过零拷贝解析 DMA 缓冲区中的 ASCII 命令，自动调用预注册的函数。

### 功能特性

- **零拷贝解析**：命令扫描和参数切分直接返回原始缓冲区指针，无内存分配
- **DMA 友好管线**：`cmd_scan` 提取命令边界 → `cmd_queue` 缓冲排队 → `cmd_parse` 执行时切分参数
- **类型安全分发**：运行时字符串签名注册，自动类型转换（int64、uint64、string、float64）
- **返回值捕获**：支持 void / int64 / char* 三种返回类型
- **嵌入式优化**：核心模块无 stdlib 依赖，尾部优先字节比较加速名称匹配
- **统一日志**：`rpc_log` 取代 printf，分级输出（DEBUG/INFO/WARN/ERROR + 协议级别），零 stdio 依赖

### 目录结构

```
CarrotInvoke/
├── inc/                 # 公开头文件
│   ├── cmdscan.h        # 零拷贝命令扫描 + 参数切分
│   ├── cmdqueue.h       # 命令队列 (环形缓冲区)
│   ├── dispatch.h       # 函数注册与查找
│   ├── invoke.h         # 调度执行引擎
│   ├── typeconv.h       # 字符串 <-> 类型化值转换
│   ├── rpclog.h        # 统一日志 (取代 printf)
│   └── ringbuf.h        # 通用环形缓冲区 (可选 DMA 同步)
├── src/                 # 实现
├── examples/            # 示例代码
├── tests/               # 单元测试 (Unity + FFF)
├── vendor/              # 第三方库
├── docs/                # 文档
│   └── rpc_usage.md     # 使用指南 (STM32 HAL 风格)
└── CMakeLists.txt       # CMake 构建配置
```

### 快速开始

```c
#include "dispatch.h"
#include "invoke.h"
#include "cmdscan.h"
#include "cmdqueue.h"
#include "rpclog.h"

/* 1. 定义处理函数 */
void LED_On(void* channel) {
    HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
}

int64_t add(void* a, void* b) {
    return *(int64_t*)a + *(int64_t*)b;
}

/* 2. 带类型签名注册 */
static dispatch_registry_t dispatcher;

dispatch_init(&dispatcher);
dispatch_reg(&dispatcher, LED_On, "LED_On(i)");
dispatch_reg(&dispatcher, add,    "add(i, i) -> i");

/* 3. 使用 rpc_log 替代 printf */
rpc_info("system ready");
rpc_error("arg mismatch: expected %d, got %d", 3, 2);

/* 4. 解析并调用 */
cmd_args_t args;
cmd_parse("add(10, 20)", 12, &args);

invoke_ret_t ret;
dispatch_status_t s = invoke_call(&dispatcher, &args, &ret);
// ret.i64 == 30
```

### 调用管线

```
DMA 缓冲区 → cmd_scan → cmd_queue → cmd_parse → dispatch + invoke → 函数执行
```

### 构建

使用 `build.bat`（自动检测编译器，使用 Ninja）：

```bat
build.bat          # 构建
build.bat run      # 构建并运行测试
build.bat demo     # 构建并运行 demo
```

或手动使用 CMake：

```bash
cmake -B build
cmake --build build
./build/carrot_tests
```

### 测试

245 个测试，覆盖 8 个测试套件：e2e、cmdscan、cmdqueue、ringbuf、typeconv、dispatch、invoke、rpc_log。

### 许可证

Apache License 2.0
