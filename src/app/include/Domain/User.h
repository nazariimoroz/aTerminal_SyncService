#pragma once
#include <stduuid/uuid.h>

namespace Domain
{
    enum class VerificationMethod : std::uint8_t
    {
        Password, Google
    };

    class User
    {
    public:
        int getId() const
        {
            return _id;
        }
        void setId(const int id)
        {
            _id = id;
        }

        const std::string& getEmail() const
        {
            return _email;
        }
        void setEmail(const std::string& email)
        {
            _email = email;
        }

        const std::vector<char>& getPassword() const
        {
            return _password;
        }
        void setPassword(std::vector<char> password)
        {
            _password = std::move(password);
        }

        void setVerificationMethod(VerificationMethod verificationMethod)
        {
            _verificationMethod = verificationMethod;
        }
        VerificationMethod getVerificationMethod() const
        {
            return _verificationMethod;
        }

    protected:
        int _id = 0;
        std::string _email;
        std::vector<char> _password;
        VerificationMethod _verificationMethod = VerificationMethod::Password;
    };
} // namespace Domain
