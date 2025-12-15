#pragma once


#include <optional>
#include <stdexcept>

#include "Domain/User.h"
#include "Port/User/UserUpdatableStorage.h"
#include "Service/MessageBus.h"

namespace Service::User
{
    template <Port::User::UserStorageC UserStorageT>
    LoginUserHandler<UserStorageT>::LoginUserHandler(Service::MessageBus& messageBus,
                                                     UserStorageT& userStorage,
                                                     Util::Crypto::PasswordHasher& passwordHasher) :
        _messageBus(messageBus), _userStorage(userStorage), _passwordHasher(passwordHasher)
    {
    }

    template <Port::User::UserStorageC UserStorageT>
    std::shared_ptr<LoginUserHandler<UserStorageT>> makeLoginUserHandler(
        Service::MessageBus& messageBus, UserStorageT& userStorage,
        Util::Crypto::PasswordHasher& passwordHasher)
    {
        using LoginUserHandlerT = LoginUserHandler<UserStorageT>;
        auto self = std::shared_ptr<LoginUserHandlerT>(new LoginUserHandlerT(messageBus, userStorage, passwordHasher));

        self->getMessageBus().template registerHandler<LoginUserCommand>(self, &LoginUserHandlerT::execute);
        return self;
    }

    template <Port::User::UserStorageC UserStorageT>
    std::expected<LoginUserCommand::Result, LoginUserCommand::Error> LoginUserHandler<
        UserStorageT>::execute(const LoginUserCommand& command)
    {
        const auto user = getUserStorage().findByEmail(command.email);
        if (!user)
        {
            return std::unexpected(InvalidEmailOrPasswordError("Invalid email or password"));
        }

        const bool passwordValid = getPasswordHasher().verify(command.rawPassword, user->getPassword());
        if (!passwordValid)
        {
            return std::unexpected(InvalidEmailOrPasswordError("Invalid email or password"));
        }

        return LoginUserResult{
            .userId = user->getId(),
        };
    }
} // namespace Service::User
