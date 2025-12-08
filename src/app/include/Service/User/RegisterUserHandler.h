#pragma once
#include <memory>
#include <string>

#include "Util/Crypto/PasswordHasher.h"

namespace Service
{
    class MessageBus;
}
namespace Poco
{
    class Logger;
}

namespace Service::User
{
    template <Port::User::UserUpdatableStorageC UserUpdatableStorageT>
    class RegisterUserHandler;

    template <Port::User::UserUpdatableStorageC UserUpdatableStorageT>
    RegisterUserHandler<UserUpdatableStorageT> makeRegisterUserHandler(
        Service::MessageBus& messageBus, UserUpdatableStorageT& userStorage,
        Util::Crypto::PasswordHasher& passwordHasher);
}

namespace Service::User
{
    struct RegisterUserResult
    {
        int userId;
    };

    struct RegisterUserCommand
    {
        std::string email;
        std::string rawPassword;

        using Result = RegisterUserResult;
        using Error = std::variant<Error::StrError, Port::User::EmailAlreadyRegisteredError>;
    };

    template <Port::User::UserUpdatableStorageC UserUpdatableStorageT>
    class RegisterUserHandler
    {
        RegisterUserHandler(Service::MessageBus& messageBus, UserUpdatableStorageT& userStorage,
                            Util::Crypto::PasswordHasher& passwordHasher);

    public:
        RegisterUserHandler(RegisterUserHandler&&);

        template <Port::User::UserUpdatableStorageC UserUpdatableStorageT>
        friend RegisterUserHandler<UserUpdatableStorageT> Service::User::makeRegisterUserHandler(
            Service::MessageBus& messageBus, UserUpdatableStorageT& userStorage,
            Util::Crypto::PasswordHasher& passwordHasher);

        std::expected<RegisterUserCommand::Result, RegisterUserCommand::Error> execute(
            const RegisterUserCommand& command);

    protected:
        Service::MessageBus& _messageBus;
        Service::MessageBus& getMessageBus() const
        {
            return _messageBus;
        }

        UserUpdatableStorageT& _userStorage;
        UserUpdatableStorageT& getUserStorage() const
        {
            return _userStorage;
        }

        Util::Crypto::PasswordHasher& _passwordHasher;
        const Util::Crypto::PasswordHasher& getPasswordHasher() const
        {
            return _passwordHasher;
        }
    };

} // namespace Service::User

#include "RegisterUserHandler.inl"
