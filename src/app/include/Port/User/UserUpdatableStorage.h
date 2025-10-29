#pragma once
#include "UserStorage.h"

namespace Port
{
    class IUnitOfWork;
}

namespace Port::User
{
    class IUserUpdatableStorage : public IUserStorage
    {
    public:
        virtual void add(const Domain::User& user) = 0;
        virtual void update(const Domain::User& user) = 0;

        virtual std::unique_ptr<IUnitOfWork> beginWork() = 0;
    };
}


