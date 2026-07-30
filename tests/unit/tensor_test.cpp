#include <gtest/gtest.h>

#include <cstdint>
#include <limits>
#include <stdexcept>
#include <utility>

#include "inferlite/core/tensor.h"

namespace inferlite {
namespace {

TEST(TensorTest, ConstructorInitializesShapeAndData) {
    const Shape shape = {2, 3};
    const TensorData data = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f};

    const Tensor tensor(shape, data);

    EXPECT_EQ(tensor.shape(), shape);
    EXPECT_EQ(tensor.data(), data);
}

TEST(TensorTest, RankReturnsCorrectValue) {
    const Shape shape = {2, 3, 4};
    const TensorData data(24, 0.0f);

    const Tensor tensor(shape, data);

    EXPECT_EQ(tensor.rank(), 3U);
}

TEST(TensorTest, SizeReturnsCorrectValue) {
    const Shape shape = {2, 3};
    const TensorData data(6, 0.0f);

    const Tensor tensor(shape, data);

    EXPECT_EQ(tensor.size(), 6U);
}

TEST(TensorTest, CopyConstructionCreatesIndependentStorage) {
    const Shape shape = {2, 3};
    const TensorData data = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f};

    const Tensor original_tensor(shape, data);
    const Tensor copied_tensor = original_tensor;

    EXPECT_EQ(copied_tensor.shape(), original_tensor.shape());
    EXPECT_EQ(copied_tensor.data(), original_tensor.data());
    EXPECT_NE(copied_tensor.data().data(), original_tensor.data().data());
}

TEST(TensorTest, MoveConstructionPreservesValue) {
    const Shape shape = {2, 3};
    const TensorData data = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f};

    Tensor original_tensor(shape, data);
    const Tensor moved_tensor = std::move(original_tensor);

    EXPECT_EQ(moved_tensor.shape(), shape);
    EXPECT_EQ(moved_tensor.data(), data);
    EXPECT_EQ(moved_tensor.rank(), 2U);
    EXPECT_EQ(moved_tensor.size(), 6U);
}

TEST(TensorTest, EmptyShapeThrowsInvalidArgument) {
    const Shape shape;
    const TensorData data;

    EXPECT_THROW((Tensor(shape, data)), std::invalid_argument);
}

TEST(TensorTest, ZeroDimensionThrowsInvalidArgument) {
    const Shape shape = {2, 0, 3};
    const TensorData data;

    EXPECT_THROW((Tensor(shape, data)), std::invalid_argument);
}

TEST(TensorTest, NegativeDimensionThrowsInvalidArgument) {
    const Shape shape = {2, -1, 3};
    const TensorData data;

    EXPECT_THROW((Tensor(shape, data)), std::invalid_argument);
}

TEST(TensorTest, TooFewDataElementsThrowsInvalidArgument) {
    const Shape shape = {2, 3};
    const TensorData data(5, 0.0f);

    EXPECT_THROW((Tensor(shape, data)), std::invalid_argument);
}

TEST(TensorTest, TooManyDataElementsThrowsInvalidArgument) {
    const Shape shape = {2, 3};
    const TensorData data(7, 0.0f);

    EXPECT_THROW((Tensor(shape, data)), std::invalid_argument);
}

TEST(TensorTest, ShapeProductOverflowThrowsOverflowError) {
    const Shape shape = {
        std::numeric_limits<std::int64_t>::max(),
        3,
    };
    const TensorData data;

    EXPECT_THROW((Tensor(shape, data)), std::overflow_error);
}

} // namespace
} // namespace inferlite