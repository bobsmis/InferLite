#pragma once

#include <stdexcept>
#include <type_traits>
#include <utility>
#include <variant>

#include "inferlite/core/status.h"

namespace inferlite {

template <typename T> class [[nodiscard]] StatusOr final {
    static_assert(!std::is_reference_v<T>, "StatusOr<T> does not support reference types");
    static_assert(!std::is_same_v<T, Status>, "StatusOr<Status> is ambiguous");

  public:
    explicit StatusOr(T value) : storage_(std::move(value)) {}

    explicit StatusOr(Status status) : storage_(ValidateErrorStatus(std::move(status))) {}

    [[nodiscard]] bool ok() const noexcept { return std::holds_alternative<T>(storage_); }

    [[nodiscard]] const Status& status() const {
        if (ok()) {
            throw std::logic_error("StatusOr::status() called on a value");
        }
        return std::get<Status>(storage_);
    }

    [[nodiscard]] const T& value() const& {
        EnsureHasValue();
        return std::get<T>(storage_);
    }

    [[nodiscard]] T& value() & {
        EnsureHasValue();
        return std::get<T>(storage_);
    }

    [[nodiscard]] T&& value() && {
        EnsureHasValue();
        return std::move(std::get<T>(storage_));
    }

  private:
    static Status ValidateErrorStatus(Status status) {
        if (status.ok()) {
            throw std::logic_error("StatusOr error state requires a non-OK Status");
        }
        return status;
    }

    void EnsureHasValue() const {
        if (!ok()) {
            throw std::logic_error("StatusOr::value() called on an error");
        }
    }

    std::variant<T, Status> storage_;
};

} // namespace inferlite
