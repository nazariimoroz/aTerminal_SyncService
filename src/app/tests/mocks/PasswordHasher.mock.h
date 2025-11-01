#pragma once
#include <gmock/gmock.h>
#include <string>
#include "Util/Crypto/PasswordHasher.h"

class MockPasswordHasher : public Util::Crypto::PasswordHasher
{
public:
    MOCK_METHOD(std::vector<char>, hash, (const std::string_view&), (const, override));
    MOCK_METHOD(bool, verify, (const std::string_view&, const std::vector<char>&), (const, override));
};
