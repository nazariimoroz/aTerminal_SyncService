#pragma once

#include "Service/User/AuthUserViaGoogleHandler.h"

#include <Poco/Logger.h>
#include <stdexcept>
#include "Util/Crypto/PasswordHasher.h"

#include "Port/UnitOfWork.h"
#include "Port/User/UserUpdatableStorage.h"
#include "Service/MessageBus.h"

namespace Service::User
{
    template <Port::User::UserUpdatableStorageC UserUpdatableStorageT>
    AuthUserViaGoogleHandler<UserUpdatableStorageT>::AuthUserViaGoogleHandler(
        Service::MessageBus& messageBus, UserUpdatableStorageT& userStorage,
        Util::Crypto::PasswordHasher& passwordHasher) :
        _messageBus(messageBus), _userStorage(userStorage), _passwordHasher(passwordHasher)
    {
    }

    template <Port::User::UserUpdatableStorageC UserUpdatableStorageT>
    AuthUserViaGoogleHandler<UserUpdatableStorageT>::AuthUserViaGoogleHandler(AuthUserViaGoogleHandler&&) =
        default;

    template <Port::User::UserUpdatableStorageC UserUpdatableStorageT>
    AuthUserViaGoogleHandler<UserUpdatableStorageT> makeAuthUserViaGoogleHandler(
        Service::MessageBus& messageBus, UserUpdatableStorageT& userStorage,
        Util::Crypto::PasswordHasher& passwordHasher)
    {
        using AuthUserViaGoogleHandlerT = AuthUserViaGoogleHandler<UserUpdatableStorageT>;
        auto self = AuthUserViaGoogleHandlerT(messageBus, userStorage, passwordHasher);

        self.getMessageBus().template registerHandler<AuthUserViaGoogleCommand>(self, &AuthUserViaGoogleHandlerT::execute);
        return self;
    }

    template <Port::User::UserUpdatableStorageC UserUpdatableStorageT>
    std::expected<AuthUserViaGoogleCommand::Result, AuthUserViaGoogleCommand::Error> AuthUserViaGoogleHandler<
        UserUpdatableStorageT>::execute(const AuthUserViaGoogleCommand& command)
    {
        auto uow = getUserStorage().beginWork();

        Domain::User user;

        auto password = getPasswordHasher().hash(command.rawPassword);
        if (!password)
        {
            uow.rollback();
            return std::unexpected(password.error());
        }

        user.setEmail(command.email);
        user.setPassword(std::move(*password));

        return getUserStorage()
            .add(user)
            .transform(
                [&]
                {
                    uow.commit();
                    return AuthUserViaGoogleResult{
                        .userId = user.getId(),
                    };
                })
            .transform_error([&](auto&& error)
            {
                uow.rollback();
                return std::forward<decltype(error)>(error);
            });
    }
} // namespace Service::User

