#include <gtest/gtest.h>
#include <httplib.h>

#include <cstdint>
#include <memory>
#include <utility>

#include "inferlite/http/http_server.h"

namespace inferlite {
namespace {
std::unique_ptr<HttpServer> CreateTestServer(std::uint16_t port = 0) {
    HttpServerOptions options;
    options.host = "127.0.0.1";
    options.port = port;

    auto result = HttpServer::Create(options);
    if (!result.ok()) {
        ADD_FAILURE() << "server creation failed: " << result.status().message();
        return nullptr;
    }

    return std::move(result).value();
}
TEST(HttpServerTest, NewlyCreatedServerIsStopped)
{
    auto server = CreateTestServer();
    ASSERT_NE(server, nullptr);
    EXPECT_FALSE(server->running());
    EXPECT_FALSE(server->bound_port().has_value());
}
    TEST(HttpServerTest, HealthRouteReturnsOk) {
    auto server = CreateTestServer();
    ASSERT_NE(server, nullptr);

    const Status status = server->Start();
    ASSERT_TRUE(status.ok()) << status.message();
    ASSERT_TRUE(server->running());

    const auto port = server->bound_port();
    ASSERT_TRUE(port.has_value());

    httplib::Client client("127.0.0.1", *port);
    const auto response = client.Get("/v1/health");

    ASSERT_TRUE(response);
    EXPECT_EQ(response->status, 200);
    EXPECT_EQ(response->body, "ok");
    EXPECT_EQ(
        response->get_header_value("Content-Type"),
        "text/plain; charset=utf-8");
}

TEST(HttpServerTest, UnknownRouteReturnsNotFound) {
    auto server = CreateTestServer();
    ASSERT_NE(server, nullptr);
    ASSERT_TRUE(server->Start().ok());

    const auto port = server->bound_port();
    ASSERT_TRUE(port.has_value());

    httplib::Client client("127.0.0.1", *port);
    const auto response = client.Get("/unknown");

    ASSERT_TRUE(response);
    EXPECT_EQ(response->status, 404);
}

TEST(HttpServerTest, RepeatedHealthRequestsSucceed) {
    auto server = CreateTestServer();
    ASSERT_NE(server, nullptr);
    ASSERT_TRUE(server->Start().ok());

    const auto port = server->bound_port();
    ASSERT_TRUE(port.has_value());

    httplib::Client client("127.0.0.1", *port);

    for (int request_index = 0; request_index < 3; ++request_index) {
        const auto response = client.Get("/v1/health");

        ASSERT_TRUE(response);
        EXPECT_EQ(response->status, 200);
        EXPECT_EQ(response->body, "ok");
    }
}

TEST(HttpServerTest, StartingRunningServerReturnsError) {
    auto server = CreateTestServer();
    ASSERT_NE(server, nullptr);
    ASSERT_TRUE(server->Start().ok());

    const Status second_start = server->Start();

    EXPECT_FALSE(second_start.ok());
    EXPECT_EQ(second_start.code(), ErrorCode::kInternal);
    EXPECT_TRUE(server->running());
}

TEST(HttpServerTest, PortConflictReturnsError) {
    auto first_server = CreateTestServer();
    ASSERT_NE(first_server, nullptr);
    ASSERT_TRUE(first_server->Start().ok());

    const auto occupied_port = first_server->bound_port();
    ASSERT_TRUE(occupied_port.has_value());

    auto second_server = CreateTestServer(*occupied_port);
    ASSERT_NE(second_server, nullptr);

    const Status status = second_server->Start();

    EXPECT_FALSE(status.ok());
    EXPECT_EQ(status.code(), ErrorCode::kInternal);
    EXPECT_FALSE(second_server->running());
    EXPECT_FALSE(second_server->bound_port().has_value());
}

TEST(HttpServerTest, StopIsIdempotentAndReleasesPort) {
    auto first_server = CreateTestServer();
    ASSERT_NE(first_server, nullptr);
    ASSERT_TRUE(first_server->Start().ok());

    const auto released_port = first_server->bound_port();
    ASSERT_TRUE(released_port.has_value());

    first_server->Stop();
    first_server->Stop();

    EXPECT_FALSE(first_server->running());
    EXPECT_FALSE(first_server->bound_port().has_value());

    auto second_server = CreateTestServer(*released_port);
    ASSERT_NE(second_server, nullptr);

    const Status status = second_server->Start();

    EXPECT_TRUE(status.ok()) << status.message();
    EXPECT_TRUE(second_server->running());
}

TEST(HttpServerTest, DestructorReleasesPort) {
    std::uint16_t released_port = 0;

    {
        auto first_server = CreateTestServer();
        ASSERT_NE(first_server, nullptr);
        ASSERT_TRUE(first_server->Start().ok());

        const auto port = first_server->bound_port();
        ASSERT_TRUE(port.has_value());
        released_port = *port;
    }

    auto second_server = CreateTestServer(released_port);
    ASSERT_NE(second_server, nullptr);

    const Status status = second_server->Start();

    EXPECT_TRUE(status.ok()) << status.message();
}

TEST(HttpServerTest, EmptyHostIsRejected) {
    HttpServerOptions options;
    options.host.clear();
    options.port = 0;

    const auto result = HttpServer::Create(options);

    ASSERT_FALSE(result.ok());
    EXPECT_EQ(result.status().code(), ErrorCode::kInvalidArgument);
}
}
}