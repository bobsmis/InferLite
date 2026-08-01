#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "inferlite/core/status_or.h"

namespace inferlite {

using Shape = std::vector<std::int64_t>;
using TensorData = std::vector<float>;

class Tensor final {
  public:
    [[nodiscard]] static StatusOr<Tensor> Create(Shape shape, TensorData data);

    [[nodiscard]] std::size_t rank() const noexcept;
    [[nodiscard]] std::size_t size() const noexcept;
    [[nodiscard]] const Shape& shape() const noexcept;
    [[nodiscard]] const TensorData& data() const noexcept;

  private:
    Tensor(Shape shape, TensorData data);

    Shape shape_;
    TensorData data_;
};

} // namespace inferlite
