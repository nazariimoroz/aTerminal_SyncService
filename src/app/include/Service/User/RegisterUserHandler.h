#pragma once
#include <memory>
#include <string>

namespace Port::User
{
    class IUserUpdatableStorage;
}
namespace Port
{
    class IUnitOfWork;
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
    };

    class RegisterUserHandler
    {
    public:
        explicit RegisterUserHandler(
            std::shared_ptr<Port::User::IUserUpdatableStorage> userStorage);

        RegisterUserResult execute(const RegisterUserCommand& command);

    protected:
        std::shared_ptr<Port::User::IUserUpdatableStorage> _userStorage;
        const std::shared_ptr<Port::User::IUserUpdatableStorage>& getUserStorage() const { return _userStorage; }
    };
} // namespace Service::User
