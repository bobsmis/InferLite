#include "inferlite/core/tensor.h"

#include <limits>
#include <stdexcept>
#include <utility>

namespace inferlite {

Tensor::Tensor(Shape shape, TensorData data) : shape_(std::move(shape)), data_(std::move(data)) {
    if (shape_.empty()) {
        throw std::invalid_argument("Tensor shape must not be empty.");
    }

    std::size_t element_count = 1;
    const std::size_t max_element_count = std::numeric_limits<std::size_t>::max();

    for (const std::int64_t dimension : shape_) {
        if (dimension <= 0) {
            throw std::invalid_argument("Tensor dimensions must be positive.");
        }

        const auto unsigned_dimension = static_cast<std::uint64_t>(dimension);

        if (unsigned_dimension > max_element_count) {
            throw std::overflow_error("Tensor dimension exceeds the size_t range.");
        }

        const auto dimension_size = static_cast<std::size_t>(unsigned_dimension);

        if (element_count > max_element_count / dimension_size) {
            throw std::overflow_error("Tensor element count overflow.");
        }

        element_count *= dimension_size;
    }

    if (data_.size() != element_count) {
        throw std::invalid_argument(
            "Tensor data size does not match the product of shape dimensions.");
    }
}

std::size_t Tensor::rank() const noexcept { return shape_.size(); }

std::size_t Tensor::size() const noexcept { return data_.size(); }

const Shape& Tensor::shape() const noexcept { return shape_; }

const TensorData& Tensor::data() const noexcept { return data_; }

} // namespace inferlite