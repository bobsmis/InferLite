#include "inferlite/core/inference_request.h"

#include <cstddef>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <vector>

#include "inferlite/core/inference_result.h"
#include "inferlite/core/status.h"

namespace inferlite {
namespace {

Status ValidateNamedTensors(const std::vector<NamedTensor>& tensors, std::string_view role,
                            std::string_view empty_collection_message) {
    if (tensors.empty()) {
        return Status::InvalidArgument(std::string(empty_collection_message));
    }

    std::unordered_set<std::string> seen_names;
    seen_names.reserve(tensors.size());

    for (std::size_t index = 0; index < tensors.size(); ++index) {
        const std::string& name = tensors[index].name;

        if (name.empty()) {
            std::ostringstream message;
            message << role << " name is empty: index=" << index;
            return Status::InvalidArgument(message.str());
        }

        const auto [iterator, inserted] = seen_names.insert(name);
        static_cast<void>(iterator);
        if (!inserted) {
            std::ostringstream message;
            message << "duplicate " << role << " name: name=" << name << ", index=" << index;
            return Status::InvalidArgument(message.str());
        }
    }

    return Status::Ok();
}

const Tensor* FindTensor(const std::vector<NamedTensor>& tensors, std::string_view name) noexcept {
    for (const NamedTensor& named_tensor : tensors) {
        const std::string_view current_name(named_tensor.name.data(), named_tensor.name.size());
        if (current_name == name) {
            return &named_tensor.tensor;
        }
    }

    return nullptr;
}

} // namespace

InferenceRequest::InferenceRequest(std::string model_name, std::vector<NamedTensor> inputs)
    : model_name_(std::move(model_name)), inputs_(std::move(inputs)) {}

StatusOr<InferenceRequest> InferenceRequest::Create(std::string model_name,
                                                    std::vector<NamedTensor> inputs) {
    if (model_name.empty()) {
        return StatusOr<InferenceRequest>(Status::InvalidArgument("model name cannot be empty"));
    }

    const Status validation =
        ValidateNamedTensors(inputs, "input", "request inputs cannot be empty");
    if (!validation.ok()) {
        return StatusOr<InferenceRequest>(validation);
    }

    return StatusOr<InferenceRequest>(InferenceRequest(std::move(model_name), std::move(inputs)));
}

std::string_view InferenceRequest::model_name() const noexcept {
    return std::string_view(model_name_.data(), model_name_.size());
}

std::size_t InferenceRequest::input_count() const noexcept { return inputs_.size(); }

const std::vector<NamedTensor>& InferenceRequest::inputs() const noexcept { return inputs_; }

const Tensor* InferenceRequest::FindInput(std::string_view name) const noexcept {
    return FindTensor(inputs_, name);
}

InferenceResult::InferenceResult(std::vector<NamedTensor> outputs) : outputs_(std::move(outputs)) {}

StatusOr<InferenceResult> InferenceResult::Create(std::vector<NamedTensor> outputs) {
    const Status validation =
        ValidateNamedTensors(outputs, "output", "result outputs cannot be empty");
    if (!validation.ok()) {
        return StatusOr<InferenceResult>(validation);
    }

    return StatusOr<InferenceResult>(InferenceResult(std::move(outputs)));
}

std::size_t InferenceResult::output_count() const noexcept { return outputs_.size(); }

const std::vector<NamedTensor>& InferenceResult::outputs() const noexcept { return outputs_; }

const Tensor* InferenceResult::FindOutput(std::string_view name) const noexcept {
    return FindTensor(outputs_, name);
}

} // namespace inferlite
