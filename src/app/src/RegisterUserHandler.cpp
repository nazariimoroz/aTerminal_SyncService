#include "Service/User/RegisterUserHandler.h"

#include <stdexcept>

#include "Port/UnitOfWork.h"
#include "Port/User/UserUpdatableStorage.h"

using namespace Service::User;

RegisterUserHandler::RegisterUserHandler(std::shared_ptr<Port::User::IUserUpdatableStorage> userStorage) :
    _userStorage(std::move(userStorage))
{
}

RegisterUserResult RegisterUserHandler::execute(const RegisterUserCommand& command)
{
    const auto us = getUserStorage();
    auto uow = us->beginWork();

    Domain::User user;
    try
    {
        user.setEmail(command.email);

        us->add(user);
    }
    catch (std::exception& exception)
    {
        uow->rollback();
        throw;
    }

    uow->commit();

    return RegisterUserResult{
        .userId = user.getId(),
    };
}
