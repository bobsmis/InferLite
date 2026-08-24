#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>

#include "inferlite/core/status.h"
#include "inferlite/core/status_or.h"

namespace inferlite {

struct HttpServerOptions final {
    std::string host{"127.0.0.1"};
    std::uint16_t port{8080};
};

class HttpServer {
  public:
    static StatusOr<std::unique_ptr<HttpServer>> Create(const HttpServerOptions& options);
    // 利用工厂模式创建传入构造参数为HttpServerOptions的StatusOr对象，返回一个指向HttpServer对象的智能指针
    // 独占指针Unique_ptr的特点：只能被一个指针拥有，不能被复制，但可以被移动
    ~HttpServer();

    HttpServer(const HttpServer&) = delete;
    HttpServer& operator=(const HttpServer&) = delete;
    HttpServer(HttpServer&&) = delete;
    HttpServer& operator=(HttpServer&&) = delete;

    // 调用者忽略启动结果时，请求编译器发出警告。
    [[nodiscard]] Status Start();
    void Stop() noexcept;
    [[nodiscard]] bool running() const noexcept;
    [[nodiscard]] std::optional<std::uint16_t> bound_port() const;

  private:
    class Impl;

    explicit HttpServer(std::unique_ptr<Impl> impl);

    std::unique_ptr<Impl> impl_;
};

} // namespace inferlite
