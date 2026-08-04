#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

#include <userver/utils/expected.hpp>

#include <modbus/exception_code.hpp>
#include <modbus/function_code.hpp>
#include <modbus/parse_error.hpp>
#include <modbus/utils.hpp>

namespace modbus::response {

class ErrorResponse {
public:
    static constexpr std::size_t kEncodedSize = 2;

    static userver::utils::expected<ErrorResponse, ParseError> Create(
        FunctionCode function_code,
        ExceptionCode exception_code
    ) noexcept {
        return ErrorResponse{function_code, exception_code};
    }

    static userver::utils::expected<ErrorResponse, ParseError> Deserialize(std::span<const std::byte>& buffer
    ) noexcept {
        if (buffer.size() < kEncodedSize) {
            return userver::utils::unexpected{ParseError::kBufferTooShort};
        }

        const auto raw_fc = static_cast<std::uint8_t>(buffer[0]);
        if (!IsErrorFunctionCode(raw_fc)) {
            return userver::utils::unexpected{ParseError::kInvalidFunctionCode};
        }

        const auto raw_ec = static_cast<std::uint8_t>(buffer[1]);
        if (!IsValidExceptionCode(raw_ec)) {
            return userver::utils::unexpected{ParseError::kInvalidExceptionCode};
        }

        buffer = buffer.subspan(kEncodedSize);

        return Create(static_cast<FunctionCode>(ToNormalFunctionCode(raw_fc)), static_cast<ExceptionCode>(raw_ec));
    }

    userver::utils::expected<std::span<std::byte>, ParseError> Serialize(std::span<std::byte> out) const noexcept {
        if (out.size() < kEncodedSize) {
            return userver::utils::unexpected{ParseError::kBufferTooShort};
        }

        out[0] = static_cast<std::byte>(ToErrorFunctionCode(function_code_));
        out[1] = static_cast<std::byte>(exception_code_);

        return out.subspan(kEncodedSize);
    }

    [[nodiscard]] FunctionCode GetFunctionCode() const noexcept { return function_code_; }

    [[nodiscard]] ExceptionCode GetExceptionCode() const noexcept { return exception_code_; }

private:
    ErrorResponse(FunctionCode function_code, ExceptionCode exception_code) noexcept
        : function_code_{function_code}, exception_code_{exception_code} {}

    FunctionCode function_code_;
    ExceptionCode exception_code_;
};

}  // namespace modbus::response
