#pragma once

#include <cstdint>

#include <modbus/exception_code.hpp>
#include <modbus/function_code.hpp>
#include <modbus/parse_error.hpp>
#include <modbus/utils.hpp>

#include <userver/utils/expected.hpp>

namespace modbus::response {

class ErrorResponse {
public:
    static userver::utils::expected<ErrorResponse, ParseError> Create(
        FunctionCode function_code,
        ExceptionCode exception_code
    ) noexcept {
        return ErrorResponse{function_code, exception_code};
    }

    template <typename InputIt>
    static userver::utils::expected<ErrorResponse, ParseError> Deserialize(InputIt& first, InputIt last) noexcept {
        const auto raw_fc = ReadBe<std::uint8_t>(first, last);
        if (!raw_fc) {
            return userver::utils::unexpected{raw_fc.error()};
        }

        if (!IsErrorFunctionCode(*raw_fc)) {
            return userver::utils::unexpected{ParseError::kInvalidFunctionCode};
        }

        const auto raw_ec = ReadBe<std::uint8_t>(first, last);
        if (!raw_ec) {
            return userver::utils::unexpected{raw_ec.error()};
        }

        if (!IsValidExceptionCode(*raw_ec)) {
            return userver::utils::unexpected{ParseError::kInvalidExceptionCode};
        }

        if (first != last) {
            return userver::utils::unexpected{ParseError::kExtraDataAtEnd};
        }

        return Create(static_cast<FunctionCode>(ToNormalFunctionCode(*raw_fc)), static_cast<ExceptionCode>(*raw_ec));
    }

    template <typename OutputIt>
    [[nodiscard]] OutputIt Serialize(OutputIt out
    ) const noexcept(noexcept(WriteBe(out, ToErrorFunctionCode(function_code_)))) {
        out = WriteBe(out, ToErrorFunctionCode(function_code_));
        out = WriteBe(out, static_cast<std::uint8_t>(exception_code_));
        return out;
    }

    [[nodiscard]] FunctionCode GetFunctionCode() const noexcept { return function_code_; }

    [[nodiscard]] ExceptionCode GetExceptionCode() const noexcept { return exception_code_; }

private:
    constexpr ErrorResponse(FunctionCode function_code, ExceptionCode exception_code) noexcept
        : function_code_{function_code}, exception_code_{exception_code} {}

    FunctionCode function_code_;
    ExceptionCode exception_code_;
};

}  // namespace modbus::response
