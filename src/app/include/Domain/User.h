#pragma once
#include <stduuid/uuid.h>

namespace Domain
{
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

        const std::string& getPassword() const
        {
            return _password;
        }
        void setPassword(const std::string& password)
        {
            _password = password;
        }

    protected:
        int _id = 0;
        std::string _email;
        std::string _password;
    };

    class UserDomainService
    {
    public:
        User RegisterUser(std::string email, std::string rawPassword)
        {
            User user;
            user.setEmail(email);
        }
    };
} // namespace Domain
