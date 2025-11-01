#include "gtest/gtest.h"
#include "Util/Crypto/PasswordHasher.h"

TEST(PasswordHasherTest, HashVerificationSuccess) {
    Util::Crypto::PasswordHasher hasher;
    std::string password = "StrongPassword123";

    const auto hashedPassword = hasher.hash(password);

    ASSERT_FALSE(hashedPassword.empty());
    ASSERT_NE(std::string(hashedPassword.begin(), hashedPassword.end()), password);

    ASSERT_TRUE(hasher.verify(password, hashedPassword));
}

TEST(PasswordHasherTest, HashVerificationFailure) {
    Util::Crypto::PasswordHasher hasher;
    std::string password = "StrongPassword123";
    std::string wrongPassword = "WrongPassword456";

    auto hashedPassword = hasher.hash(password);

    ASSERT_FALSE(hasher.verify(wrongPassword, hashedPassword));
}