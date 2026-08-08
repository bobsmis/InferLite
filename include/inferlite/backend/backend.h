#pragma once

#include "inferlite/core/inference_request.h"
#include "inferlite/core/inference_result.h"
#include "inferlite/core/status_or.h"

namespace inferlite {

class IBackend {
  public:
    virtual ~IBackend() = default;

    [[nodiscard]] virtual StatusOr<InferenceResult> Infer(const InferenceRequest& request) = 0;
};

} // namespace inferlite
