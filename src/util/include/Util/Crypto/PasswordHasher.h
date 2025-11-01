#pragma once
#include <string>
#include <string_view>
#include <vector>

namespace Util::Crypto
{
    class PasswordHasher
    {
    public:
        PasswordHasher();

        virtual std::vector<char> hash(const std::string_view& password) const;
        virtual bool verify(const std::string_view& password, const std::vector<char>& hashed_password) const;
    };
}