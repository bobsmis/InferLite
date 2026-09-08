#pragma once

#include <string>
#include <string_view>

#include "inferlite/core/inference_request.h"
#include "inferlite/core/inference_result.h"
#include "inferlite/core/status_or.h"

namespace inferlite {

[[nodiscard]] StatusOr<InferenceRequest> ParseInferenceRequestJson(std::string_view json_text);

[[nodiscard]] StatusOr<std::string> SerializeInferenceResultJson(const InferenceResult& result);

} // namespace inferlite
