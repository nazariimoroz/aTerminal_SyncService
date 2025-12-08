#include "Util/Crypto/PasswordHasher.h"

#include <Poco/Exception.h>
#include <stdexcept>

#include "Util/LogicException.h"
#include "sodium.h"

using namespace Util::Crypto;

PasswordHasher::PasswordHasher()
{
    if (sodium_init() < 0)
    {
        throw Util::LogicException("Failed to initialize sodium");
    }
}

std::expected<std::vector<char>, Error::StrError> PasswordHasher::hash(const std::string_view& password) const
{
    std::vector<char> toRet(crypto_pwhash_STRBYTES, ' ');

    if (crypto_pwhash_str(
            toRet.data(),
            password.data(),
            password.size(),
            crypto_pwhash_OPSLIMIT_MODERATE,
            crypto_pwhash_MEMLIMIT_MODERATE
        ) != 0)
    {
        return std::unexpected(Error::StrError("Failed to hash password"));
    }

    return toRet;
}

bool PasswordHasher::verify(const std::string_view& password, const std::vector<char>& hashed_password) const
{
    return crypto_pwhash_str_verify(
        hashed_password.data(),
        password.data(),
        password.size()) == 0;
}
