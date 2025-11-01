#pragma once
#include "UserStorage.h"
#include "Util/BusinessException.h"

namespace Port
{
    class IUnitOfWork;
}

namespace Port::User
{
    POCO_DECLARE_EXCEPTION(, EmailAlreadyRegisteredException, Util::BusinessException)

    class IUserUpdatableStorage : public IUserStorage
    {
    public:
        ~IUserUpdatableStorage() override;

        virtual void add(Domain::User& user) = 0;
        virtual void update(const Domain::User& user) = 0;

        virtual std::unique_ptr<IUnitOfWork> beginWork() = 0;
    };
}


