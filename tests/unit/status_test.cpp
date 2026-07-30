#include <gtest/gtest.h>

#include <utility>

#include "inferlite/core/status.h"

namespace inferlite {
namespace {

TEST(StatusTest, DefaultStatusIsOk) {
    const Status status;

    EXPECT_TRUE(status.ok());
    EXPECT_EQ(status.code(), ErrorCode::kOk);
    EXPECT_TRUE(status.message().empty());
}

TEST(StatusTest, OkFactoryCreatesSuccessfulStatus) {
    const Status status = Status::Ok();

    EXPECT_TRUE(status.ok());
    EXPECT_EQ(status.code(), ErrorCode::kOk);
    EXPECT_TRUE(status.message().empty());
}

TEST(StatusTest, InvalidArgumentCarriesCodeAndMessage) {
    const Status status = Status::InvalidArgument("shape is invalid");

    EXPECT_FALSE(status.ok());
    EXPECT_EQ(status.code(), ErrorCode::kInvalidArgument);
    EXPECT_EQ(status.message(), "shape is invalid");
}

TEST(StatusTest, NotFoundCarriesCodeAndMessage) {
    const Status status = Status::NotFound("model was not found");

    EXPECT_FALSE(status.ok());
    EXPECT_EQ(status.code(), ErrorCode::kNotFound);
    EXPECT_EQ(status.message(), "model was not found");
}

TEST(StatusTest, InternalCarriesCodeAndMessage) {
    const Status status = Status::Internal("backend failure");

    EXPECT_FALSE(status.ok());
    EXPECT_EQ(status.code(), ErrorCode::kInternal);
    EXPECT_EQ(status.message(), "backend failure");
}

TEST(StatusTest, CanBeCopied) {
    const Status original = Status::InvalidArgument("invalid request");
    const Status copied = original;

    EXPECT_EQ(copied.code(), ErrorCode::kInvalidArgument);
    EXPECT_EQ(copied.message(), "invalid request");
}

TEST(StatusTest, CanBeMoved) {
    Status original = Status::Internal("internal error");
    const Status moved = std::move(original);

    EXPECT_EQ(moved.code(), ErrorCode::kInternal);
    EXPECT_EQ(moved.message(), "internal error");
}

}  // namespace
}  // namespace inferlite