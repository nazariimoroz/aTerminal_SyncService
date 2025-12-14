#pragma once
#include <string>
#include <string_view>

namespace Error
{
    struct StrError
    {
        StrError() = default;
        explicit StrError(std::string_view inErrorMessage) : errorMessage(std::move(inErrorMessage)) {}

        std::string_view errorMessage;
    };

    struct MutStrError
    {
        MutStrError() = default;
        explicit MutStrError(const StrError& error) : errorMessage(error.errorMessage) {}
        explicit MutStrError(std::string inErrorMessage) : errorMessage(std::move(inErrorMessage)) {}

        std::string errorMessage;
    };
}
