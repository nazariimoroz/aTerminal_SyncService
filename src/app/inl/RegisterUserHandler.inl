#pragma once


namespace Service::User
{
    template <Port::User::UserUpdatableStorageC UserUpdatableStorageT>
    RegisterUserHandler<UserUpdatableStorageT>::RegisterUserHandler(
        Service::MessageBus& messageBus, UserUpdatableStorageT& userStorage,
        Util::Crypto::PasswordHasher& passwordHasher) :
        _messageBus(messageBus), _userStorage(userStorage), _passwordHasher(passwordHasher)
    {
    }

    template <Port::User::UserUpdatableStorageC UserUpdatableStorageT>
    RegisterUserHandler<UserUpdatableStorageT>::RegisterUserHandler(RegisterUserHandler&&) =
        default;

    template <Port::User::UserUpdatableStorageC UserUpdatableStorageT>
    std::shared_ptr<RegisterUserHandler<UserUpdatableStorageT>> makeRegisterUserHandler(
        Service::MessageBus& messageBus, UserUpdatableStorageT& userStorage,
        Util::Crypto::PasswordHasher& passwordHasher)
    {
        using RegisterUserHandlerT = RegisterUserHandler<UserUpdatableStorageT>;
        auto self = std::shared_ptr<RegisterUserHandlerT>(new RegisterUserHandlerT(messageBus, userStorage, passwordHasher));

        self->getMessageBus().template registerHandler<RegisterUserCommand>(self, &RegisterUserHandlerT::execute);
        return self;
    }

    template <Port::User::UserUpdatableStorageC UserUpdatableStorageT>
    std::expected<RegisterUserCommand::Result, RegisterUserCommand::Error> RegisterUserHandler<
        UserUpdatableStorageT>::execute(const RegisterUserCommand& command)
    {
        auto uow = getUserStorage().beginWork();

        Domain::User user;

        auto password = getPasswordHasher().hash(command.rawPassword);
        if (!password)
        {
            uow.rollback();
            return std::unexpected(password.error());
        }

        user.setEmail(command.email);
        user.setPassword(std::move(*password));

        return getUserStorage()
            .add(user)
            .transform(
                [&]
                {
                    uow.commit();
                    return RegisterUserResult{
                        .userId = user.getId(),
                    };
                })
            .transform_error([&](auto&& error)
            {
                uow.rollback();
                return std::forward<decltype(error)>(error);
            });
    }
} // namespace Service::User
