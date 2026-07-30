#include "inferlite/core/status.h"

#include <utility>

namespace inferlite {

Status::Status(ErrorCode code, std::string message)
    : code_(code), message_(std::move(message)) {}

Status Status::Ok() noexcept {
    return Status{};
}

Status Status::InvalidArgument(std::string message) {
    return Status{ErrorCode::kInvalidArgument, std::move(message)};
}

Status Status::NotFound(std::string message) {
    return Status{ErrorCode::kNotFound, std::move(message)};
}

Status Status::Internal(std::string message) {
    return Status{ErrorCode::kInternal, std::move(message)};
}

bool Status::ok() const noexcept {
    return code_ == ErrorCode::kOk;
}

ErrorCode Status::code() const noexcept {
    return code_;
}

const std::string& Status::message() const noexcept {
    return message_;
}

}  // namespace inferlite