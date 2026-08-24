#include "inferlite/http/http_server.h"

#include <httplib.h>

#include <atomic>
#include <thread>
#include <utility>

namespace inferlite {

class HttpServer::Impl final { // 参数校验、错误转换、状态管理、注册推理业务回调
  public:
explicit Impl(HttpServerOptions options)
    : options_(std::move(options)) {
    server_.set_socket_options([](socket_t socket) {
        httplib::set_socket_opt(
            socket,
            SOL_SOCKET,
            SO_REUSEADDR,
            1);
    });

    RegisterRoutes();
}


    Status Start() {
        if (server_.is_running()) {
            return Status::Internal("HTTP server is already running");
        }

        if (options_.port == 0) {
            int assigned_port = server_.bind_to_any_port(options_.host);
            if (assigned_port < 0) {
                return Status::Internal("Failed to bind to any port" +
                                        std::to_string(options_.port));
            }
            bound_port_ = assigned_port;
        } else {
            if (!server_.bind_to_port(options_.host, options_.port)) {
                return Status::Internal("Failed to bind to port " + std::to_string(options_.port));
            }
            bound_port_ = options_.port;
        }
        listener_thread_ = std::thread([this]() {
            // 这个函数会无限阻塞，直到调用 server_.stop() 才会返回
            server_.listen_after_bind();
        });
        server_.wait_until_ready();

        // 检查是否真正运行起来
        if (server_.is_running()) {
            return Status::Ok();
        } else {
            server_.stop(); // 通知httplib终止监听循环，唤醒阻塞在listen_after_bind的子线程
            if (listener_thread_.joinable()) { // 判断线程是否可join（线程还没join过）
                listener_thread_.join(); // 主线程等待子线程执行完毕，回收线程资源，防止僵尸线程
            }
            bound_port_.store(-1, std::memory_order_release);
            return Status::Internal("http server listen start failed");
        }
    }
    void Stop() noexcept {
        server_.stop(); // 通知httplib终止监听循环，唤醒阻塞在listen_after_bind的子线程
        if (listener_thread_.joinable()) { // 线程是否需要回收与服务器当前是否运行是两个独立状态
            listener_thread_.join(); // 主线程等待子线程执行完毕，回收线程资源
        }
        bound_port_.store(-1, std::memory_order_release);
    }

    bool running() const noexcept { return server_.is_running(); }
    std::optional<std::uint16_t> bound_port() const // 获取http服务实际成功的端口号
    {
        int port = bound_port_.load(std::memory_order_acquire);
        if (port == -1) {
            return std::nullopt; // 返回：没有端口，空
        }
        return static_cast<std::uint16_t>(port); // 返回：有端口值
    }

  private:
    void RegisterRoutes() // 注册路由
    {
        server_.Get("/v1/health", [](const httplib::Request&, httplib::Response& res) {
            res.status = 200;
            res.set_content("ok", "text/plain; charset=utf-8");
        });
    }
    HttpServerOptions options_;
    httplib::Server server_;
    std::thread listener_thread_; // 监听线程

    std::atomic<int> bound_port_{-1};
};

// HttpServer公开成员函数的实现

StatusOr<std::unique_ptr<HttpServer>> HttpServer::Create(const HttpServerOptions& options) {
    if (options.host.empty()) {
return StatusOr<std::unique_ptr<HttpServer>>(
    Status::InvalidArgument("host is empty"));
    }

    auto impl = std::make_unique<Impl>(options);
    return StatusOr<std::unique_ptr<HttpServer>>(
        std::unique_ptr<HttpServer>(new HttpServer(std::move(impl))));
}

HttpServer::HttpServer(std::unique_ptr<Impl> impl) : impl_(std::move(impl)) {}
HttpServer::~HttpServer() { Stop(); }
Status HttpServer::Start() { return impl_->Start(); }
void HttpServer::Stop() noexcept { impl_->Stop(); }
bool HttpServer::running() const noexcept { return impl_->running(); }
std::optional<std::uint16_t> HttpServer::bound_port() const { return impl_->bound_port(); }
} // namespace inferlite