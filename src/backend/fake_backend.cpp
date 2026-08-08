#include "inferlite/backend/fake_backend.h"

#include "inferlite/core/status.h"

namespace inferlite {

FakeBackend::FakeBackend(FakeBackendMode mode) noexcept : mode_(mode) {}

StatusOr<InferenceResult> FakeBackend::Infer(const InferenceRequest& request) {
    if (mode_ == FakeBackendMode::kForcedFailure) {
        return StatusOr<InferenceResult>(Status::Internal("fake backend forced failure"));
    }

    return InferenceResult::Create(request.inputs());
}

} // namespace inferlite
