#include "Service/User/RegisterUserHandler.h"

#include <stdexcept>

#include "Port/User/UserUpdatableStorage.h"

using namespace Service::User;

RegisterUserHandler::RegisterUserHandler(std::shared_ptr<Port::User::IUserUpdatableStorage> UserStorage) :
    _userStorage(std::move(UserStorage))
{
}

RegisterUserResult RegisterUserHandler::execute(const RegisterUserCommand& Command)
{
    const auto us = getUserStorage();
    auto uow = us->beginWork();

    Domain::User

    us->add()

    return RegisterUserResult{};
}
