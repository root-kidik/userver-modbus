#pragma once

#include <cstdint>

#include <userver/modbus/exception_code.hpp>
#include <userver/modbus/function_code.hpp>
#include <userver/modbus/parse_error.hpp>
#include <userver/modbus/utils.hpp>

#include <userver/utils/expected.hpp>

namespace modbus::response {

class ErrorResponse {
public:
    static constexpr std::size_t kSerializedSize{2};

    static userver::utils::expected<ErrorResponse, ParseError> Create(
        FunctionCode function_code,
        ExceptionCode exception_code
    ) noexcept {
        const auto raw_fc = static_cast<std::uint8_t>(function_code);

        const auto error_fc =
            IsErrorFunctionCode(raw_fc) ? function_code : static_cast<FunctionCode>(ToErrorFunctionCode(raw_fc));

        return ErrorResponse{error_fc, exception_code};
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

        if (first != last) {
            return userver::utils::unexpected{ParseError::kExtraDataAtEnd};
        }

        return ErrorResponse{static_cast<FunctionCode>(*raw_fc), static_cast<ExceptionCode>(*raw_ec)};
    }

    template <typename OutputIt>
    [[nodiscard]] OutputIt Serialize(OutputIt out
    ) const noexcept(noexcept(WriteBe(out, static_cast<std::uint8_t>(function_code_)))) {
        out = WriteBe(out, static_cast<std::uint8_t>(function_code_));
        out = WriteBe(out, static_cast<std::uint8_t>(exception_code_));
        return out;
    }

    [[nodiscard]] FunctionCode GetFunctionCode() const noexcept { return function_code_; }

    [[nodiscard]] ExceptionCode GetExceptionCode() const noexcept { return exception_code_; }

    [[nodiscard]] FunctionCode GetOriginalFunctionCode() const noexcept {
        const auto raw_fc = static_cast<std::uint8_t>(function_code_);
        return static_cast<FunctionCode>(ToNormalFunctionCode(raw_fc));
    }

private:
    constexpr ErrorResponse(FunctionCode function_code, ExceptionCode exception_code) noexcept
        : function_code_{function_code}, exception_code_{exception_code} {}

    FunctionCode function_code_;
    ExceptionCode exception_code_;
};

}  // namespace modbus::response
