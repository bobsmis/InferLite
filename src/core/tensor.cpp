#include "inferlite/core/tensor.h"

#include <cstddef>
#include <cstdint>
#include <limits>
#include <sstream>
#include <utility>

namespace inferlite {
namespace {

StatusOr<std::size_t> ComputeElementCount(const Shape& shape) {
    const auto max_size = static_cast<std::uintmax_t>(std::numeric_limits<std::size_t>::max());
    bool has_zero_dimension = false;

    for (std::size_t index = 0; index < shape.size(); ++index) {
        const std::int64_t dimension = shape[index];

        if (dimension < 0) {
            std::ostringstream message;
            message << "Tensor dimension is negative: index=" << index << ", value=" << dimension;
            return StatusOr<std::size_t>(Status::InvalidArgument(message.str()));
        }

        if (dimension == 0) {
            has_zero_dimension = true;
            continue;
        }

        const auto unsigned_dimension = static_cast<std::uintmax_t>(dimension);
        if (unsigned_dimension > max_size) {
            std::ostringstream message;
            message << "Tensor dimension exceeds size_t range: index=" << index
                    << ", value=" << dimension;
            return StatusOr<std::size_t>(Status::InvalidArgument(message.str()));
        }
    }

    if (has_zero_dimension) {
        return StatusOr<std::size_t>(std::size_t{0});
    }

    std::size_t element_count = 1;
    const std::size_t max_element_count = std::numeric_limits<std::size_t>::max();

    for (std::size_t index = 0; index < shape.size(); ++index) {
        const auto dimension = static_cast<std::size_t>(shape[index]);

        if (element_count > max_element_count / dimension) {
            std::ostringstream message;
            message << "Tensor element count overflow: index=" << index
                    << ", value=" << shape[index] << ", partial_product=" << element_count;
            return StatusOr<std::size_t>(Status::InvalidArgument(message.str()));
        }

        element_count *= dimension;
    }

    return StatusOr<std::size_t>(element_count);
}

} // namespace

Tensor::Tensor(Shape shape, TensorData data) : shape_(std::move(shape)), data_(std::move(data)) {}

StatusOr<Tensor> Tensor::Create(Shape shape, TensorData data) {
    StatusOr<std::size_t> element_count = ComputeElementCount(shape);
    if (!element_count.ok()) {
        return StatusOr<Tensor>(element_count.status());
    }

    const std::size_t expected_size = element_count.value();
    if (data.size() != expected_size) {
        std::ostringstream message;
        message << "Tensor data size mismatch: expected=" << expected_size
                << ", actual=" << data.size();
        return StatusOr<Tensor>(Status::InvalidArgument(message.str()));
    }

    return StatusOr<Tensor>(Tensor(std::move(shape), std::move(data)));
}

std::size_t Tensor::rank() const noexcept { return shape_.size(); }

std::size_t Tensor::size() const noexcept { return data_.size(); }

const Shape& Tensor::shape() const noexcept { return shape_; }

const TensorData& Tensor::data() const noexcept { return data_; }

} // namespace inferlite
