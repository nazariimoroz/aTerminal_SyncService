#pragma once
#include <expected>
#include <string>
#include <string_view>
#include <vector>

#include "Util/Errors.h"

namespace Util::Crypto
{
    class PasswordHasher
    {
    public:
        PasswordHasher();

        virtual std::expected<std::vector<char>, Error::StrError> hash(const std::string_view& password) const;
        virtual bool verify(const std::string_view& password, const std::vector<char>& hashed_password) const;
    };
}