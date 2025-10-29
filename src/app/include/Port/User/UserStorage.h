#pragma once

#include <concepts>
#include <optional>
#include <string>

#include "Domain/User.h"

namespace Port::User
{
    class IUserStorage
    {
    public:
        virtual ~IUserStorage();

        virtual std::optional<Domain::User> findById(const uuids::uuid& userId) = 0;
        virtual std::optional<Domain::User> findByEmail(const std::string& email) = 0;
        virtual bool existsByEmail(const std::string& userId) = 0;
    };
}
