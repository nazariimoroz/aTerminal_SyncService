#pragma once
#include <memory>
#include <string>

#include "Port/User/UserUpdatableStorage.h"
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
    class AuthUserViaGoogleHandler;

    template <Port::User::UserUpdatableStorageC UserUpdatableStorageT>
    AuthUserViaGoogleHandler<UserUpdatableStorageT> makeAuthUserViaGoogleHandler(
        Service::MessageBus& messageBus, UserUpdatableStorageT& userStorage,
        Util::Crypto::PasswordHasher& passwordHasher);
}

namespace Service::User
{
    struct AuthUserViaGoogleResult
    {
        int userId;
    };

    struct AuthUserViaGoogleCommand
    {
        std::string code;
        std::string code_verifier;

        using Result = AuthUserViaGoogleResult;
        using Error = std::variant<Error::StrError, Port::User::EmailAlreadyRegisteredError>;
    };

    template <Port::User::UserUpdatableStorageC UserUpdatableStorageT>
    class AuthUserViaGoogleHandler
    {
        AuthUserViaGoogleHandler(Service::MessageBus& messageBus, UserUpdatableStorageT& userStorage,
                            Util::Crypto::PasswordHasher& passwordHasher);

    public:
        AuthUserViaGoogleHandler(AuthUserViaGoogleHandler&&);

        template <Port::User::UserUpdatableStorageC InnerUserUpdatableStorageT>
        friend AuthUserViaGoogleHandler<InnerUserUpdatableStorageT> Service::User::makeAuthUserViaGoogleHandler(
            Service::MessageBus& messageBus, InnerUserUpdatableStorageT& userStorage,
            Util::Crypto::PasswordHasher& passwordHasher);

        std::expected<AuthUserViaGoogleCommand::Result, AuthUserViaGoogleCommand::Error> execute(
            const AuthUserViaGoogleCommand& command);

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

#include "AuthUserViaGoogleHandler.inl"
