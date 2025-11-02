#pragma once

#include <Poco/Exception.h>
#include <memory>
#include <string>

#include "Util/BusinessException.h"
#include "Util/Crypto/PasswordHasher.h"

namespace Service
{
    class MessageBus;
}
namespace Port::User
{
    class IUserStorage;
}

namespace Service::User
{
    POCO_DECLARE_EXCEPTION(, InvalidEmailOrPasswordException, Util::BusinessException)

    struct LoginUserResult
    {
        int userId;
    };

    struct LoginUserCommand
    {
        std::string email;
        std::string rawPassword;

        using Result = LoginUserResult;
    };

    class LoginUserHandler
    {
        LoginUserHandler(std::shared_ptr<Service::MessageBus> messageBus,
                         std::shared_ptr<Port::User::IUserStorage> userStorage,
                         std::shared_ptr<Util::Crypto::PasswordHasher> passwordHasher);

    public:
        static std::shared_ptr<LoginUserHandler> make(std::shared_ptr<Service::MessageBus> messageBus,
                                                      std::shared_ptr<Port::User::IUserStorage> userStorage,
                                                      std::shared_ptr<Util::Crypto::PasswordHasher> passwordHasher);

        LoginUserResult execute(const LoginUserCommand& command);

    protected:
        std::shared_ptr<Service::MessageBus> _messageBus;
        const std::shared_ptr<Service::MessageBus>& getMessageBus() const
        {
            return _messageBus;
        }

        std::shared_ptr<Port::User::IUserStorage> _userStorage;
        const std::shared_ptr<Port::User::IUserStorage>& getUserStorage() const
        {
            return _userStorage;
        }

        std::shared_ptr<Util::Crypto::PasswordHasher> _passwordHasher;
        const std::shared_ptr<Util::Crypto::PasswordHasher>& getPasswordHasher() const
        {
            return _passwordHasher;
        }
    };

} // namespace Service::User
