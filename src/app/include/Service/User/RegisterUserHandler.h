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
        std::string UserId;
        std::string Email;
    };

    struct RegisterUserCommand
    {
        std::string Email;
        std::string RawPassword;

        using Result = RegisterUserResult;
    };

    class RegisterUserHandler
    {
    public:
        explicit RegisterUserHandler(
            std::shared_ptr<Port::User::IUserUpdatableStorage> UserStorage);

        RegisterUserResult execute(const RegisterUserCommand& Command);

    protected:
        std::shared_ptr<Port::User::IUserUpdatableStorage> _userStorage;
        const std::shared_ptr<Port::User::IUserUpdatableStorage>& getUserStorage() const { return _userStorage; }
    };
} // namespace Service::User
