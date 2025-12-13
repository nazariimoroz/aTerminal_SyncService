#pragma once

#include <Poco/Exception.h>
#include <string>

#include "Port/User/UserUpdatableStorage.h"
#include "Util/Crypto/PasswordHasher.h"

namespace Service
{
    class MessageBus;
}

namespace Service::User
{
    template <Port::User::UserStorageC UserStorageT>
    class LoginUserHandler;

    template <Port::User::UserStorageC UserStorageT>
    LoginUserHandler<UserStorageT> makeLoginUserHandler(
        Service::MessageBus& messageBus, UserStorageT& userStorage,
        Util::Crypto::PasswordHasher& passwordHasher);
}

namespace Service::User
{
    struct InvalidEmailOrPasswordError : Error::StrError
    {
        using StrError::StrError;
    };

    struct LoginUserResult
    {
        int userId;
    };

    struct LoginUserCommand
    {
        std::string email;
        std::string rawPassword;

        using Result = LoginUserResult;
        using Error = InvalidEmailOrPasswordError;
    };

    template <Port::User::UserStorageC UserStorageT>
    class LoginUserHandler
    {
        LoginUserHandler(Service::MessageBus& messageBus, UserStorageT& userStorage,
                         Util::Crypto::PasswordHasher& passwordHasher);

    public:
        LoginUserHandler(LoginUserHandler&&);

        template <Port::User::UserStorageC InnerUserStorageT>
        friend LoginUserHandler<InnerUserStorageT> Service::User::makeLoginUserHandler(
            Service::MessageBus& messageBus, InnerUserStorageT& userStorage,
            Util::Crypto::PasswordHasher& passwordHasher);

        std::expected<LoginUserCommand::Result, LoginUserCommand::Error> execute(
            const LoginUserCommand& command);

    protected:
        Service::MessageBus& _messageBus;
        Service::MessageBus& getMessageBus() const
        {
            return _messageBus;
        }

        UserStorageT& _userStorage;
        UserStorageT& getUserStorage() const
        {
            return _userStorage;
        }

        Util::Crypto::PasswordHasher& _passwordHasher;
        Util::Crypto::PasswordHasher& getPasswordHasher() const
        {
            return _passwordHasher;
        }
    };

} // namespace Service::User

#include "LoginUserHandler.inl"
