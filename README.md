# InferLite

InferLite 是一个使用 C++17 构建的轻量级单机 CPU 模型推理服务学习项目。项目通过可验证功能单元逐步实现请求校验、Backend 抽象、HTTP 服务、ONNX Runtime 推理、异步执行、背压、动态批处理和性能观测。

## 当前状态

第一个同步推理闭环 `v0.0.1` 的代码与自动化门禁已经完成，当前等待学习验收与版本提交。

已完成：

- `ErrorCode`、`Status`和`StatusOr<T>`；
- 经过 Shape、长度和溢出校验的拥有式 float32 `Tensor`；
- 标量和零元素 Tensor 语义；
- `NamedTensor`；
- `InferenceRequest`与`InferenceResult`；
- 输入输出名称校验、顺序保持和只读查找；
- `IBackend`统一同步推理接口；
- 支持确定性回显和失败注入的`FakeBackend`；
- 串联Tensor、Request、Backend和Result的命令行程序；
- 覆盖成功、Backend失败和参数错误的CLI黑盒测试；
- Debug、Release、ASan 和 UBSan 构建入口；
- GoogleTest 与 CTest 自动化测试。

当前自动化证据：

- Debug 全量测试：49/49；
- Release 全量测试：49/49；
- ASan/UBSan 全量测试：49/49；
- APP-01 `clang-format --dry-run --Werror`通过。

CLI闭环验收后进入HTTP + FakeBackend功能链。

## 当前调用链

```text
Tensor::Create
    ↓
NamedTensor
    ↓
InferenceRequest
    ↓
IBackend / FakeBackend
    ↓
InferenceResult
    ↓
CLI输出、错误流和退出码
```

## 核心所有权约定

- `Tensor`拥有 Shape 和连续 `vector<float>`数据；
- `NamedTensor`拥有名称和 Tensor；
- `InferenceRequest`拥有模型名及有序输入集合；
- `InferenceResult`拥有有序输出集合；
- 调用者通过`unique_ptr<IBackend>`拥有具体Backend；
- Backend只在`Infer`调用期间借用Request，并返回拥有式Result；
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
model=demo_model
output name=x shape=[2] data=[1,2]
```

验证Backend失败路径：

```bash
./build/debug/apps/inferlite_demo --fail
```

标准错误输出：

```text
inference failed: fake backend forced failure
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
└── tests/
    ├── integration/
    └── unit/
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
- CLI当前使用固定的演示输入；
- FakeBackend只用于验证调用链和错误传播，不执行真实模型计算；
- HTTP、ONNX Runtime、异步队列和动态Batch尚未进入；
- 当前没有性能数据；
- 项目定位为学习和实验性实现，不使用未经验证的生产级表述。
