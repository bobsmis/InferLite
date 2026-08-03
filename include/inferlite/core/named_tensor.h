#pragma once

#include <string>

#include "inferlite/core/tensor.h"

namespace inferlite {

struct NamedTensor final {
    std::string name;
    Tensor tensor;
};

} // namespace inferlite
