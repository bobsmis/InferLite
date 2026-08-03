# InferLite

InferLite 是一个使用 C++17 构建的轻量级单机 CPU 模型推理服务学习项目。项目通过可验证功能单元逐步实现请求校验、Backend 抽象、HTTP 服务、ONNX Runtime 推理、异步执行、背压、动态批处理和性能观测。

## 当前状态

当前正在形成第一个同步推理闭环 `v0.0.1`。

已完成：

- `ErrorCode`、`Status`和`StatusOr<T>`；
- 经过 Shape、长度和溢出校验的拥有式 float32 `Tensor`；
- 标量和零元素 Tensor 语义；
- `NamedTensor`；
- `InferenceRequest`与`InferenceResult`；
- 输入输出名称校验、顺序保持和只读查找；
- Debug、Release、ASan 和 UBSan 构建入口；
- GoogleTest 与 CTest 自动化测试。

当前自动化证据：

- Debug 全量测试：43/43；
- ASan/UBSan 全量测试：43/43；
- CORE-03 Release 目标测试：17/17；
- CORE-03 `clang-format --dry-run --Werror`通过。

下一功能单元是 `IBackend`与`FakeBackend`，随后由 CLI 串联 Request → Backend → Result，形成 `v0.0.1`。

## 当前调用链

```text
Tensor::Create
    ↓
NamedTensor
    ↓
InferenceRequest
    ↓
IBackend / FakeBackend（下一单元）
    ↓
InferenceResult
    ↓
CLI（v0.0.1）
```

## 核心所有权约定

- `Tensor`拥有 Shape 和连续 `vector<float>`数据；
- `NamedTensor`拥有名称和 Tensor；
- `InferenceRequest`拥有模型名及有序输入集合；
- `InferenceResult`拥有有序输出集合；
- `FindInput`和`FindOutput`返回非拥有的 `const Tensor*`，其生命周期受 Request 或 Result 约束；
- 可预期的输入错误通过 `StatusOr<T>`传播。

## 环境

- Linux / WSL2 Ubuntu 24.04
- C++17
- CMake 3.25+
- Ninja
- GCC 或 Clang
- GoogleTest

## 项目Target

- `inferlite_core`：核心静态库；
- `inferlite_demo`：命令行程序；
- `inferlite_tests`：GoogleTest 测试程序。

## 使用CMake Presets

### Debug

```bash
cmake --preset debug
cmake --build --preset debug
ctest --preset debug
```

### Release

```bash
cmake --preset release
cmake --build --preset release
ctest --preset release
```

### ASan与UBSan

```bash
cmake --preset sanitize
cmake --build --preset sanitize
ctest --preset sanitize
```

## 运行Demo

```bash
./build/debug/apps/inferlite_demo
```

当前Demo输出：

```text
InferLite
```

## 目录结构

```text
InferLite/
├── apps/
├── cmake/
├── docs/
├── include/inferlite/
│   ├── backend/
│   └── core/
├── src/
│   ├── backend/
│   └── core/
└── tests/unit/
```

## 版本路线

```text
v0.0.1  FakeBackend命令行闭环
v0.1    HTTP + FakeBackend
v0.2    ONNX Runtime同步推理
v0.3    有界队列、异步执行与背压
v0.4    动态批处理
v0.5    指标与性能报告
v1.0    CI、文档、Benchmark和发布收口
```

项目技术深度将重点体现在：

- Tensor与 `Ort::Value`的内存生命周期；
- 多生产者、多消费者队列和TSan验证；
- Deadline、容量拒绝与优雅关闭；
- Batch合并、拆分和等待窗口调度；
- 同步、异步和动态Batch的QPS与P50/P95/P99对照。

## 当前限制

- 仅支持拥有式连续float32 Tensor；
- Backend和CLI同步推理闭环尚在开发；
- HTTP、ONNX Runtime、异步队列和动态Batch尚未进入；
- 当前没有性能数据；
- 项目定位为学习和实验性实现，不使用未经验证的生产级表述。
