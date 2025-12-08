#pragma once
#include <string_view>

namespace Error
{
    struct StrError
    {
        StrError() = default;
        StrError(std::string_view inErrorMessage) : errorMessage(std::move(inErrorMessage)) {}

        std::string_view errorMessage;
    };
}
