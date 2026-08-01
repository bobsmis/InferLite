#include <gtest/gtest.h>

#include <memory>
#include <stdexcept>
#include <utility>

#include "inferlite/core/status_or.h"

namespace inferlite {
namespace {

TEST(StatusOrTest, ValueResultIsOk) {
    const StatusOr<int> result(42);

    EXPECT_TRUE(result.ok());
    EXPECT_EQ(result.value(), 42);
}

TEST(StatusOrTest, ErrorResultPreservesStatus) {
    const StatusOr<int> result(Status::InvalidArgument("invalid value"));

    ASSERT_FALSE(result.ok());
    EXPECT_EQ(result.status().code(), ErrorCode::kInvalidArgument);
    EXPECT_EQ(result.status().message(), "invalid value");
}

TEST(StatusOrTest, OkStatusCannotCreateErrorResult) {
    EXPECT_THROW((StatusOr<int>(Status::Ok())), std::logic_error);
}

TEST(StatusOrTest, ValueOnErrorThrowsLogicError) {
    const StatusOr<int> result(Status::Internal("backend failure"));

    EXPECT_THROW((void)result.value(), std::logic_error);
}

TEST(StatusOrTest, StatusOnValueThrowsLogicError) {
    const StatusOr<int> result(42);

    EXPECT_THROW((void)result.status(), std::logic_error);
}

TEST(StatusOrTest, MovesValueOutOfRvalueResult) {
    StatusOr<std::unique_ptr<int>> result(std::make_unique<int>(7));

    std::unique_ptr<int> value = std::move(result).value();

    ASSERT_NE(value, nullptr);
    EXPECT_EQ(*value, 7);
}

} // namespace
} // namespace inferlite
