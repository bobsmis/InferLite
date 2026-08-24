# InferLite

InferLite 是一个使用 C++17 构建的轻量级单机 CPU 模型推理服务学习项目。项目按“接口、实现、测试、构建与验证”组成的功能单元推进，目标是在可复现的工程证据上逐步完成 HTTP 服务、ONNX Runtime 推理、异步背压、动态批处理、性能诊断与交付体系。

## 当前状态

- `v0.0.1` FakeBackend 命令行闭环：已完成；
- `HTTP-01` Server 生命周期与健康检查：已完成；
- 当前单元：`HTTP-02` JSON 与 Request/Result 转换，待开始；
- 增强版 `v1.0` 功能单元进度：6/37，约 16%。

当前已形成两条可运行链路：

```text
CLI
 └── Tensor → InferenceRequest → IBackend/FakeBackend → InferenceResult

HTTP Client
 └── HttpServer → GET /v1/health → HTTP 200 + "ok"
```

## 已实现功能

### Core

- `ErrorCode`、`Status`和`StatusOr<T>`统一错误传播；
- 拥有式连续 float32 `Tensor`；
- Shape、数据长度、负维度和乘法溢出校验；
- 标量与零元素 Tensor 语义；
- `NamedTensor`、`InferenceRequest`与`InferenceResult`；
- 输入输出名称校验、顺序保持、复制移动和只读查找。

### Backend与CLI

- `IBackend`同步推理抽象接口；
- 支持确定性回显和失败注入的`FakeBackend`；
- 串联Tensor、Request、Backend和Result的命令行程序；
- 成功、Backend失败和非法参数三类CLI黑盒路径。

### HTTP

- 使用PImpl封装cpp-httplib，避免第三方类型进入公开头文件；
- RAII管理Server、Socket和监听线程生命周期；
- 支持固定端口和由操作系统选择动态端口；
- `GET /v1/health`健康检查；
- 端口冲突检测、重复停止和析构释放端口；
- SIGINT/SIGTERM停止Server进程；
- `SO_REUSEADDR`与Linux端口复用语义验证。

## 自动化证据

当前共注册58项测试：

- Debug：58/58；
- Release：58/58；
- ASan/UBSan：58/58；
- HTTP集成测试：9/9；
- `clang-format --dry-run --Werror`通过；
- `git diff --check`通过。

测试分层包括：

- Core和Backend单元测试；
- 使用真实Socket、客户端和监听线程的HTTP集成测试；
- 检查stdout、stderr和退出码的CLI黑盒测试。

## 技术栈

- **语言与设计**：C++17、RAII、智能指针、移动语义、值语义、模板、STL、运行时多态、PImpl；
- **构建**：现代CMake、CMake Presets、Ninja、FetchContent、多Target依赖管理；
- **测试与质量**：GoogleTest、CTest、ASan、UBSan、clang-format、编译警告；
- **服务与Linux**：cpp-httplib、HTTP、TCP端口生命周期、`std::thread`、SIGINT/SIGTERM、`curl`、`ss`；
- **协作**：Git、GitHub、功能单元式开发与可验证提交。

## Target与依赖关系

```text
inferlite_demo
└── PRIVATE inferlite_core

inferlite_server
└── PRIVATE inferlite_http

inferlite_http
├── PUBLIC  inferlite_core
└── PRIVATE httplib::httplib

inferlite_tests
├── inferlite_core
├── inferlite_http
├── httplib::httplib
└── GTest::gtest_main
```

主要Target：

- `inferlite_core`：Tensor、Request/Result、Status和Backend静态库；
- `inferlite_http`：HTTP适配与Server生命周期静态库；
- `inferlite_demo`：FakeBackend命令行闭环；
- `inferlite_server`：可运行健康检查服务；
- `inferlite_tests`：GoogleTest测试程序。

## 环境

- Linux / WSL2 Ubuntu 24.04
- C++17
- CMake 3.25+
- Ninja
- GCC或Clang
- Git

第三方依赖由CMake FetchContent接入：

- GoogleTest v1.17.0
- cpp-httplib v0.51.0

## 构建与测试

### Debug

```bash
cmake --preset debug
cmake --build --preset debug
ctest --preset debug --output-on-failure
```

### Release

```bash
cmake --preset release
cmake --build --preset release
ctest --preset release --output-on-failure
```

### ASan与UBSan

```bash
cmake --preset sanitize
cmake --build --preset sanitize
ctest --preset sanitize --output-on-failure
```

只运行HTTP测试：

```bash
./build/debug/tests/inferlite_tests --gtest_filter='HttpServerTest.*'
```

## 运行CLI Demo

正常路径：

```bash
./build/debug/apps/inferlite_demo
```

输出示例：

```text
model=demo_model
output name=x shape=[2] data=[1,2]
```

Backend失败注入：

```bash
./build/debug/apps/inferlite_demo --fail
```

标准错误输出：

```text
inference failed: fake backend forced failure
```

## 运行HTTP Server

终端一：

```bash
./build/debug/apps/inferlite_server
```

输出：

```text
HTTP server is running on http://127.0.0.1:8080
```

终端二：

```bash
curl -i http://127.0.0.1:8080/v1/health
```

预期响应：

```text
HTTP/1.1 200 OK
Content-Type: text/plain; charset=utf-8

ok
```

在Server终端按`Ctrl+C`后，进程处理SIGINT，停止监听线程并释放端口：

```text
HTTP server stopped
```

## 核心所有权约定

- `Tensor`拥有Shape和连续`vector<float>`数据；
- `NamedTensor`拥有名称和Tensor；
- `InferenceRequest`拥有模型名及有序输入集合；
- `InferenceResult`拥有有序输出集合；
- 调用者通过`unique_ptr<IBackend>`拥有具体Backend；
- Backend只在`Infer`调用期间借用Request，并返回拥有式Result；
- `FindInput`和`FindOutput`返回非拥有的`const Tensor*`；
- `unique_ptr<HttpServer>`拥有Server，Server通过`unique_ptr<Impl>`拥有实现；
- `Impl`负责停止httplib、回收监听线程和清理绑定端口；
- 可预期的构造或调用错误通过`Status/StatusOr<T>`传播。

## 目录结构

```text
InferLite/
├── apps/
│   ├── demo_main.cpp
│   └── server_main.cpp
├── cmake/
├── docs/
├── include/inferlite/
│   ├── backend/
│   ├── core/
│   └── http/
├── src/
│   ├── backend/
│   ├── core/
│   └── http/
└── tests/
    ├── integration/
    └── unit/
```

## 路线图

```text
v0.0.1  FakeBackend命令行闭环                         已完成
v0.1    HTTP、JSON、FakeBackend同步服务               进行中
v0.2    ONNX Runtime真实同步推理
v0.3    有界队列、异步执行、背压和优雅关闭
v0.4    动态Batch、指标和首轮性能报告
v0.5    Linux网络实验、Protobuf与gRPC
v0.6    MySQL请求审计与Redis结果缓存
v0.7    内存模型实验、Linux性能诊断与Docker部署
v1.0    CI、文档、全量门禁与Release收口
```

后续技术深度重点：

- Tensor与`Ort::Value`之间的内存生命周期；
- 有界队列、多生产者/消费者、TSan和背压；
- Deadline、容量拒绝与优雅关闭；
- Batch合并、拆分和等待窗口调度；
- QPS与P50/P95/P99实验；
- epoll、Protobuf/gRPC、MySQL、Redis、perf和Docker。

## 当前限制

- 仅支持拥有式连续float32 Tensor；
- CLI使用固定演示输入；
- FakeBackend只验证调用链和错误传播，不执行真实模型计算；
- HTTP当前只提供`GET /v1/health`；
- JSON、`POST /v1/infer`和ONNX Runtime尚未接入；
- 异步队列、动态Batch和性能数据仍在后续功能单元；
- 项目目前是学习与实验性实现，所有能力以已提交代码和测试证据为准。
