#include "Service/User/LoginUserHandler.h"

#include <optional>
#include <stdexcept>

#include "Domain/User.h"
#include "Port/User/UserUpdatableStorage.h"
#include "Service/MessageBus.h"

using namespace Service::User;

POCO_IMPLEMENT_EXCEPTION(InvalidEmailOrPasswordException, Util::BusinessException, "InvalidEmailOrPasswordException")

LoginUserHandler::LoginUserHandler(std::shared_ptr<Service::MessageBus> messageBus,
                                   std::shared_ptr<Port::User::IUserStorage> userStorage,
                                   std::shared_ptr<Util::Crypto::PasswordHasher> passwordHasher) :
    _messageBus(std::move(messageBus)), _userStorage(std::move(userStorage)), _passwordHasher(std::move(passwordHasher))
{
}

std::shared_ptr<LoginUserHandler> LoginUserHandler::make(std::shared_ptr<Service::MessageBus> messageBus,
                                                         std::shared_ptr<Port::User::IUserStorage> userStorage,
                                                         std::shared_ptr<Util::Crypto::PasswordHasher> passwordHasher)
{
    auto self = std::shared_ptr<LoginUserHandler>(
        new LoginUserHandler(std::move(messageBus), std::move(userStorage), std::move(passwordHasher)));

    self->getMessageBus()->registerHandler<LoginUserCommand>(self, &LoginUserHandler::execute);
    return self;
}

LoginUserResult LoginUserHandler::execute(const LoginUserCommand& command)
{
    const auto us = getUserStorage();
    const auto hasher = getPasswordHasher();

    const auto user = us->findByEmail(command.email);
    if (!user)
    {
        throw InvalidEmailOrPasswordException("Invalid email or password");
    }

    const bool passwordValid = hasher->verify(command.rawPassword, user->getPassword());
    if (!passwordValid)
    {
        throw InvalidEmailOrPasswordException("Invalid email or password");
    }

    return LoginUserResult{
        .userId = user->getId(),
    };
}
