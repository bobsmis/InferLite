#include <cstddef>
#include <iostream>
#include <memory>
#include <string_view>
#include <utility>
#include <vector>

#include "inferlite/backend/backend.h"
#include "inferlite/backend/fake_backend.h"
#include "inferlite/core/inference_request.h"
#include "inferlite/core/inference_result.h"
#include "inferlite/core/named_tensor.h"
#include "inferlite/core/status_or.h"
#include "inferlite/core/tensor.h"

namespace {

constexpr int kSuccessExitCode = 0;
constexpr int kRuntimeErrorExitCode = 1;
constexpr int kUsageErrorExitCode = 2;

void PrintShape(std::ostream& output, const inferlite::Shape& shape) {
    output << '[';
    for (std::size_t index = 0; index < shape.size(); ++index) {
        if (index != 0) {
            output << ',';
        }
        output << shape[index];
    }
    output << ']';
}

void PrintData(std::ostream& output, const inferlite::TensorData& data) {
    output << '[';
    for (std::size_t index = 0; index < data.size(); ++index) {
        if (index != 0) {
            output << ',';
        }
        output << data[index];
    }
    output << ']';
}

void PrintResult(std::ostream& output, const inferlite::InferenceRequest& request,
                 const inferlite::InferenceResult& result) {
    output << "model=" << request.model_name() << '\n';
    for (const inferlite::NamedTensor& named_output : result.outputs()) {
        output << "output name=" << named_output.name << " shape=";
        PrintShape(output, named_output.tensor.shape());
        output << " data=";
        PrintData(output, named_output.tensor.data());
        output << '\n';
    }
}

} // namespace

int main(int argc, char* argv[]) {
    bool force_failure = false;
    if (argc == 2 && std::string_view(argv[1]) == "--fail") {
        force_failure = true;
    } else if (argc != 1) {
        std::cerr << "usage: inferlite_demo [--fail]\n";
        return kUsageErrorExitCode;
    }

    inferlite::StatusOr<inferlite::Tensor> tensor_result =
        inferlite::Tensor::Create({2}, {1.0F, 2.0F});
    if (!tensor_result.ok()) {
        std::cerr << "tensor creation failed: " << tensor_result.status().message() << '\n';
        return kRuntimeErrorExitCode;
    }

    std::vector<inferlite::NamedTensor> inputs;
    inputs.push_back(inferlite::NamedTensor{"x", std::move(tensor_result).value()});

    inferlite::StatusOr<inferlite::InferenceRequest> request_result =
        inferlite::InferenceRequest::Create("demo_model", std::move(inputs));
    if (!request_result.ok()) {
        std::cerr << "request creation failed: " << request_result.status().message() << '\n';
        return kRuntimeErrorExitCode;
    }

    const inferlite::FakeBackendMode mode = force_failure
                                                ? inferlite::FakeBackendMode::kForcedFailure
                                                : inferlite::FakeBackendMode::kEcho;
    std::unique_ptr<inferlite::IBackend> backend = std::make_unique<inferlite::FakeBackend>(mode);

    inferlite::StatusOr<inferlite::InferenceResult> inference_result =
        backend->Infer(request_result.value());
    if (!inference_result.ok()) {
        std::cerr << "inference failed: " << inference_result.status().message() << '\n';
        return kRuntimeErrorExitCode;
    }

    PrintResult(std::cout, request_result.value(), inference_result.value());
    return kSuccessExitCode;
}
