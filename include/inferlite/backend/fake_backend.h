#pragma once

#include "inferlite/backend/backend.h"

namespace inferlite {

enum class FakeBackendMode {
    kEcho,
    kForcedFailure,
};

class FakeBackend final : public IBackend {
  public:
    explicit FakeBackend(FakeBackendMode mode) noexcept;

    [[nodiscard]] StatusOr<InferenceResult> Infer(const InferenceRequest& request) override;

  private:
    FakeBackendMode mode_;
};

} // namespace inferlite
