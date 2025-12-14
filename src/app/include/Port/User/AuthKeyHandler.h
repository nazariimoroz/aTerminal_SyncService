#pragma once

#include <concepts>
#include <expected>
#include <optional>
#include <string>

#include "Domain/User.h"
#include "Util/Errors.h"

namespace Port::User
{
    using AuthKeyHandlerError = Error::MutStrError;

    struct AuthKeyHandlerResult
    {
        std::string email;
    };

    template <class T>
    concept AuthKeyHandlerC = requires {
        typename T::KeyDtoT;
    } && requires(const T& t, typename T::KeyDtoT keyDto) {
        { t.handle(keyDto) } -> std::same_as<std::expected<AuthKeyHandlerResult, AuthKeyHandlerError>>;
    };
} // namespace Port::User
