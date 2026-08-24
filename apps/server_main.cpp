#include "inferlite/core/status.h"
#include "inferlite/http/http_server.h"

#include <chrono>
#include <csignal>
#include <iostream>
#include <thread>
#include <utility>

namespace {

volatile std::sig_atomic_t stop_requested = 0;

void HandleSignal(int) {
    stop_requested = 1;
}

} // namespace

int main() {
    inferlite::HttpServerOptions options;

    auto server_result = inferlite::HttpServer::Create(options);
    if (!server_result.ok()) {
        std::cerr << "Failed to create HTTP server: "
                  << server_result.status().message() << '\n';
        return 1;
    }

    auto server = std::move(server_result).value();

    const inferlite::Status start_status = server->Start();
    if (!start_status.ok()) {
        std::cerr << "Failed to start HTTP server: "
                  << start_status.message() << '\n';
        return 1;
    }

    const auto port = server->bound_port();
    if (!port.has_value()) {
        std::cerr << "HTTP server has no bound port\n";
        server->Stop();
        return 1;
    }

    std::signal(SIGINT, HandleSignal);
    std::signal(SIGTERM, HandleSignal);

    std::cout << "HTTP server is running on http://"
              << options.host << ':' << *port << '\n';

    while (stop_requested == 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    server->Stop();
    std::cout << "HTTP server stopped\n";
    return 0;
}