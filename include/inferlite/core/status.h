#pragma once

#include <string>

namespace inferlite {

enum class ErrorCode {
    kOk = 0,
    kInvalidArgument,
    kNotFound,
    kInternal,
};

class [[nodiscard]] Status final {
public:
    Status() noexcept = default;

    static Status Ok() noexcept;
    static Status InvalidArgument(std::string message);
    static Status NotFound(std::string message);
    static Status Internal(std::string message);

    [[nodiscard]] bool ok() const noexcept;
    [[nodiscard]] ErrorCode code() const noexcept;
    [[nodiscard]] const std::string& message() const noexcept;

private:
    Status(ErrorCode code, std::string message);

    ErrorCode code_{ErrorCode::kOk};
    std::string message_;
};

}  // namespace inferlite