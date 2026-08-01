#include <gtest/gtest.h>

#include <cstdint>
#include <limits>
#include <string>
#include <utility>

#include "inferlite/core/tensor.h"

namespace inferlite {
namespace {

TEST(TensorTest, CreatesOrdinaryTensorFromLvalues) {
    const Shape shape = {2, 3};
    const TensorData data = {1.0F, 2.0F, 3.0F, 4.0F, 5.0F, 6.0F};

    StatusOr<Tensor> result = Tensor::Create(shape, data);

    ASSERT_TRUE(result.ok());
    const Tensor& tensor = result.value();
    EXPECT_EQ(tensor.shape(), shape);
    EXPECT_EQ(tensor.data(), data);
    EXPECT_EQ(tensor.rank(), 2U);
    EXPECT_EQ(tensor.size(), 6U);
}

TEST(TensorTest, EmptyShapeCreatesScalar) {
    StatusOr<Tensor> result = Tensor::Create({}, {42.0F});

    ASSERT_TRUE(result.ok());
    EXPECT_TRUE(result.value().shape().empty());
    EXPECT_EQ(result.value().rank(), 0U);
    EXPECT_EQ(result.value().size(), 1U);
    EXPECT_EQ(result.value().data(), TensorData({42.0F}));
}

TEST(TensorTest, EmptyShapeRequiresOneDataElement) {
    const StatusOr<Tensor> result = Tensor::Create({}, {});

    ASSERT_FALSE(result.ok());
    EXPECT_EQ(result.status().code(), ErrorCode::kInvalidArgument);
    EXPECT_NE(result.status().message().find("expected=1"), std::string::npos);
    EXPECT_NE(result.status().message().find("actual=0"), std::string::npos);
}

TEST(TensorTest, ZeroDimensionCreatesEmptyTensor) {
    StatusOr<Tensor> result = Tensor::Create({2, 0, 3}, {});

    ASSERT_TRUE(result.ok());
    EXPECT_EQ(result.value().shape(), Shape({2, 0, 3}));
    EXPECT_EQ(result.value().rank(), 3U);
    EXPECT_EQ(result.value().size(), 0U);
    EXPECT_TRUE(result.value().data().empty());
}

TEST(TensorTest, ZeroElementShapeRejectsNonemptyData) {
    const StatusOr<Tensor> result = Tensor::Create({2, 0, 3}, {1.0F});

    ASSERT_FALSE(result.ok());
    EXPECT_NE(result.status().message().find("expected=0"), std::string::npos);
    EXPECT_NE(result.status().message().find("actual=1"), std::string::npos);
}

TEST(TensorTest, NegativeDimensionReportsIndexAndValue) {
    const StatusOr<Tensor> result = Tensor::Create({2, -1, 3}, {});

    ASSERT_FALSE(result.ok());
    EXPECT_EQ(result.status().code(), ErrorCode::kInvalidArgument);
    EXPECT_NE(result.status().message().find("index=1"), std::string::npos);
    EXPECT_NE(result.status().message().find("value=-1"), std::string::npos);
}

TEST(TensorTest, NegativeDimensionIsReportedEvenAfterZeroDimension) {
    const StatusOr<Tensor> result = Tensor::Create({0, -1}, {});

    ASSERT_FALSE(result.ok());
    EXPECT_NE(result.status().message().find("index=1"), std::string::npos);
    EXPECT_NE(result.status().message().find("value=-1"), std::string::npos);
}

TEST(TensorTest, TooFewDataElementsReportsExpectedAndActualSizes) {
    const StatusOr<Tensor> result = Tensor::Create({2, 3}, TensorData(5, 0.0F));

    ASSERT_FALSE(result.ok());
    EXPECT_NE(result.status().message().find("expected=6"), std::string::npos);
    EXPECT_NE(result.status().message().find("actual=5"), std::string::npos);
}

TEST(TensorTest, TooManyDataElementsReportsExpectedAndActualSizes) {
    const StatusOr<Tensor> result = Tensor::Create({2, 3}, TensorData(7, 0.0F));

    ASSERT_FALSE(result.ok());
    EXPECT_NE(result.status().message().find("expected=6"), std::string::npos);
    EXPECT_NE(result.status().message().find("actual=7"), std::string::npos);
}

TEST(TensorTest, ShapeProductOverflowReturnsInvalidArgument) {
    const Shape shape = {
        std::numeric_limits<std::int64_t>::max(),
        3,
    };

    const StatusOr<Tensor> result = Tensor::Create(shape, {});

    ASSERT_FALSE(result.ok());
    EXPECT_EQ(result.status().code(), ErrorCode::kInvalidArgument);
    EXPECT_NE(result.status().message().find("overflow"), std::string::npos);
    EXPECT_NE(result.status().message().find("index=1"), std::string::npos);
    EXPECT_NE(result.status().message().find("value=3"), std::string::npos);
    EXPECT_NE(result.status().message().find("partial_product="), std::string::npos);
}

TEST(TensorTest, CopyConstructionCreatesIndependentStorage) {
    StatusOr<Tensor> result = Tensor::Create({2, 3}, {1.0F, 2.0F, 3.0F, 4.0F, 5.0F, 6.0F});
    ASSERT_TRUE(result.ok());

    const Tensor copied_tensor = result.value();

    EXPECT_EQ(copied_tensor.shape(), result.value().shape());
    EXPECT_EQ(copied_tensor.data(), result.value().data());
    EXPECT_NE(copied_tensor.data().data(), result.value().data().data());
}

TEST(TensorTest, MovesTensorOutOfStatusOr) {
    const Shape expected_shape = {2, 2};
    const TensorData expected_data = {1.0F, 2.0F, 3.0F, 4.0F};
    StatusOr<Tensor> result = Tensor::Create(expected_shape, expected_data);
    ASSERT_TRUE(result.ok());

    const Tensor moved_tensor = std::move(result).value();

    EXPECT_EQ(moved_tensor.shape(), expected_shape);
    EXPECT_EQ(moved_tensor.data(), expected_data);
    EXPECT_EQ(moved_tensor.size(), 4U);
}

} // namespace
} // namespace inferlite
