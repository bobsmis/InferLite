#pragma once

#include <cstddef>
#include <string_view>
#include <vector>

#include "inferlite/core/named_tensor.h"
#include "inferlite/core/status_or.h"

namespace inferlite {

class InferenceResult final {
  public:
    [[nodiscard]] static StatusOr<InferenceResult> Create(std::vector<NamedTensor> outputs);

    [[nodiscard]] std::size_t output_count() const noexcept;
    [[nodiscard]] const std::vector<NamedTensor>& outputs() const noexcept;
    [[nodiscard]] const Tensor* FindOutput(std::string_view name) const noexcept;

  private:
    explicit InferenceResult(std::vector<NamedTensor> outputs);

    std::vector<NamedTensor> outputs_;
};

} // namespace inferlite
