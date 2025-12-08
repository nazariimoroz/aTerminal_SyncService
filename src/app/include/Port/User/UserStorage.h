#pragma once

#include <concepts>
#include <optional>
#include <string>

#include "Domain/User.h"

namespace Port::User
{
    template<class T>
    concept UserStorageC = requires(T t, int userId, const std::string& email) {
        { t.findById(userId) } -> std::same_as<std::optional<Domain::User>>;
        { t.findByEmail(email) } -> std::same_as<std::optional<Domain::User>>;
        { t.existsByEmail(email) } -> std::same_as<bool>;
    };
}
