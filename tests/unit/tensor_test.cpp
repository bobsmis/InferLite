#include <gtest/gtest.h>

#include <utility>

#include "inferlite/core/tensor.h"

namespace inferlite {
namespace {

TEST(TensorTest, ConstructorInitializesShapeAndData) {
    const inferlite::Shape shape = {2, 3};
    const inferlite::TensorData data = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f};

    const inferlite::Tensor tensor(shape, data);

    EXPECT_EQ(tensor.shape(), shape);
    EXPECT_EQ(tensor.data(), data);
}

TEST(TensorTest, RankReturnsCorrectValue) {
    const inferlite::Shape shape = {2, 3, 4};
    const inferlite::TensorData data(24, 0.0f);

    const inferlite::Tensor tensor(shape, data);

    EXPECT_EQ(tensor.rank(), shape.size());
}

TEST(TensorTest, SizeReturnsCorrectValue) {
    const inferlite::Shape shape = {2, 3};
    const inferlite::TensorData data(6, 0.0f);

    const inferlite::Tensor tensor(shape, data);

    EXPECT_EQ(tensor.size(), data.size());
}

TEST(TensorTest, CopyConstruction) {
    const inferlite::Shape shape = {2, 3};
    const inferlite::TensorData data = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f};

    const inferlite::Tensor original_tensor(shape, data);
    const inferlite::Tensor copied_tensor = original_tensor;

    EXPECT_EQ(copied_tensor.shape(), shape);
    EXPECT_EQ(copied_tensor.data(), data);
}

TEST(TensorTest, MoveConstruction) {
    const inferlite::Shape shape = {2, 3};
    const inferlite::TensorData data = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f};

    inferlite::Tensor original_tensor(shape, data);
    const inferlite::Tensor moved_tensor = std::move(original_tensor);

    EXPECT_EQ(moved_tensor.shape(), shape);
    EXPECT_EQ(moved_tensor.data(), data);
    EXPECT_EQ(moved_tensor.rank(), 2);
    EXPECT_EQ(moved_tensor.size(), 6);
}
} // namespace

} // namespace inferlite