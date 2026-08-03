#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

#include "inferlite/core/named_tensor.h"
#include "inferlite/core/status_or.h"

namespace inferlite {

class InferenceRequest final {
  public:
    [[nodiscard]] static StatusOr<InferenceRequest> Create(std::string model_name,
                                                           std::vector<NamedTensor> inputs);

    [[nodiscard]] std::string_view model_name() const noexcept;
    [[nodiscard]] std::size_t input_count() const noexcept;
    [[nodiscard]] const std::vector<NamedTensor>& inputs() const noexcept;
    [[nodiscard]] const Tensor* FindInput(std::string_view name) const noexcept;

  private:
    InferenceRequest(std::string model_name, std::vector<NamedTensor> inputs);

    std::string model_name_;
    std::vector<NamedTensor> inputs_;
};

} // namespace inferlite
