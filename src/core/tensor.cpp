#include "inferlite/core/tensor.h"

#include <utility>

namespace inferlite {

Tensor::Tensor(Shape shape, TensorData data) : shape_(std::move(shape)), data_(std::move(data)) {}

std::size_t Tensor::rank() const noexcept { return shape_.size(); }

std::size_t Tensor::size() const noexcept { return data_.size(); }

const Shape& Tensor::shape() const noexcept { return shape_; }

const TensorData& Tensor::data() const noexcept { return data_; }

} // namespace inferlite