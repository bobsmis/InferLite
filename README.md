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



## 项目Target

- `inferlite_core`：核心静态库；
- `inferlite_demo`：最小命令行程序，依赖`inferlite_core`。

## Debug构建


cmake -S . -B build/debug -G Ninja \
    -DCMAKE_BUILD_TYPE=Debug \
    -DBUILD_TESTING=ON

cmake --build build/debug
ctest --test-dir build/debug --output-on-failure


## Release构建


cmake -S . -B build/release -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_TESTING=ON

cmake --build build/release
ctest --test-dir build/release --output-on-failure


## Sanitizer构建


cmake -S . -B build/sanitize -G Ninja \
    -DCMAKE_BUILD_TYPE=Debug \
    -DBUILD_TESTING=ON \
    -DINFERLITE_ENABLE_SANITIZERS=ON

cmake --build build/sanitize
ctest --test-dir build/sanitize --output-on-failure


## 运行Demo

## 运行测试

## Target说明

## 当前限制

## 短期路线
