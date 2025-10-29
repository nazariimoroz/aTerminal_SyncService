#pragma once
#include <stduuid/uuid.h>

namespace Domain
{
    class User
    {
    public:
        const uuids::uuid& getId() const
        {
            return _id;
        }
        void setId(const uuids::uuid& id)
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

    protected:
        uuids::uuid _id;
        std::string _email;
    };
} // namespace Domain
