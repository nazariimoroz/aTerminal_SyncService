#pragma once
#include <expected>
#include "Defines.h"
#include "Port/UnitOfWork.h"
#include "UserStorage.h"
#include "Util/Errors.h"

namespace Port::User
{
    struct EmailAlreadyRegisteredError : Error::StrError
    {
        using StrError::StrError;
    };

    template<class T>
    concept UserUpdatableStorageC =
        UserStorageC<T> &&
        requires(T t, Domain::User& userMut, const Domain::User& userConst) {
            { t.add(userMut) } -> std::same_as<std::expected<void, EmailAlreadyRegisteredError>>;
            { t.update(userConst) } -> std::same_as<void>;
            { t.beginWork() } -> Port::UnitOfWorkC;
        };
}


