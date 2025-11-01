#include "Service/User/RegisterUserHandler.h"

#include <Poco/Logger.h>
#include <stdexcept>

#include "Port/UnitOfWork.h"
#include "Port/User/UserUpdatableStorage.h"
#include "Service/MessageBus.h"
#include "Util/BusinessException.h"

using namespace Service::User;

RegisterUserHandler::RegisterUserHandler(std::shared_ptr<Service::MessageBus> messageBus,
                                         std::shared_ptr<Port::User::IUserUpdatableStorage> userStorage,
                                         std::shared_ptr<Util::Crypto::PasswordHasher> passwordHasher) :
    _messageBus(std::move(messageBus)), _userStorage(std::move(userStorage)), _passwordHasher(std::move(passwordHasher))
{}

std::shared_ptr<RegisterUserHandler> RegisterUserHandler::make(
    std::shared_ptr<Service::MessageBus> messageBus,
    std::shared_ptr<Port::User::IUserUpdatableStorage> userStorage,
    std::shared_ptr<Util::Crypto::PasswordHasher> passwordHasher)
{
    auto self = std::shared_ptr<RegisterUserHandler>(new RegisterUserHandler(
        std::move(messageBus),
        std::move(userStorage),
        std::move(passwordHasher)
    ));

    self->getMessageBus()->registerHandler<RegisterUserCommand>(self, &RegisterUserHandler::execute);
    return self;
}

RegisterUserResult RegisterUserHandler::execute(const RegisterUserCommand& command)
{
    const auto us = getUserStorage();
    auto uow = us->beginWork();

    Domain::User user;
    try
    {
        auto password = getPasswordHasher()->hash(command.rawPassword);

        user.setEmail(command.email);
        user.setPassword(std::move(password));

        us->add(user);
    }
    catch (Util::BusinessException&)
    {
        uow->rollback();
        throw;
    }

    uow->commit();

    return RegisterUserResult {
        .userId = user.getId(),
    };
}
