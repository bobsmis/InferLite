# InferLite

InferLite 是一个用于学习 C++ 服务端工程和模型推理系统的轻量级推理服务。

## 当前状态

项目处于 M0 工程准入阶段。

当前仅完成最小 CMake 命令行程序，尚未实现 Tensor、Backend、HTTP 和 ONNX Runtime。

## 环境

- Linux / WSL2
- C++17
- CMake
- Ninja

## Debug 构建

```bash
cmake -S . -B build/debug -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build/debug