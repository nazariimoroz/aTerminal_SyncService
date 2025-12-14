#pragma once
#include <string>
#include <rfl/Rename.hpp>

#include "Port/User/AuthKeyHandler.h"
#include "Port/User/UserUpdatableStorage.h"

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
    struct AuthUserViaGoogleResult
    {
        int userId;
        bool bAlreadyRegistered;
    };

    struct AuthUserViaGoogleCommand
    {
        std::string code;
        rfl::Rename<"code_verifier", std::string> codeVerifier;
        rfl::Rename<"redirect_uri", std::string> redirectUri;

        using Result = AuthUserViaGoogleResult;
        using Error = Error::MutStrError;
    };

    template <Port::User::AuthKeyHandlerC AuthKeyHandlerT,
              Port::User::UserUpdatableStorageC UserUpdatableStorageT>
    class AuthUserViaGoogleHandler;

    template <Port::User::AuthKeyHandlerC AuthKeyHandlerT,
              Port::User::UserUpdatableStorageC UserUpdatableStorageT>
    AuthUserViaGoogleHandler<AuthKeyHandlerT, UserUpdatableStorageT> makeAuthUserViaGoogleHandler(
        Service::MessageBus& messageBus, UserUpdatableStorageT& userStorage,
        AuthKeyHandlerT& authKeyHandler)
    {
        using AuthUserViaGoogleHandlerT = AuthUserViaGoogleHandler<AuthKeyHandlerT, UserUpdatableStorageT>;
        auto self = AuthUserViaGoogleHandlerT(messageBus, userStorage, authKeyHandler);

        self.getMessageBus().template registerHandler<AuthUserViaGoogleCommand>(
            self, &AuthUserViaGoogleHandlerT::execute);
        return self;
    }

    template <Port::User::AuthKeyHandlerC AuthKeyHandlerT,
              Port::User::UserUpdatableStorageC UserUpdatableStorageT>
    class AuthUserViaGoogleHandler
    {
        AuthUserViaGoogleHandler(Service::MessageBus& messageBus,
                                 UserUpdatableStorageT& userStorage,
                                 AuthKeyHandlerT& authKeyHandler) :
            _messageBus(messageBus), _userStorage(userStorage), _authKeyHandler(authKeyHandler)
        {
        }

    public:
        AuthUserViaGoogleHandler(AuthUserViaGoogleHandler&&) = default;

        template <Port::User::AuthKeyHandlerC InnerAuthKeyHandlerT,
                  Port::User::UserUpdatableStorageC InnerUserUpdatableStorageT>
        friend AuthUserViaGoogleHandler<InnerAuthKeyHandlerT, InnerUserUpdatableStorageT> Service::
            User::makeAuthUserViaGoogleHandler(Service::MessageBus& messageBus,
                                               InnerUserUpdatableStorageT& userStorage,
                                               InnerAuthKeyHandlerT& authKeyHandler);

    public:
        std::expected<AuthUserViaGoogleCommand::Result, AuthUserViaGoogleCommand::Error> execute(
        const AuthUserViaGoogleCommand& command)
        {
            /** Handle auth keys */
            typename AuthKeyHandlerT::KeyDtoT keyDto;
            keyDto.code = command.code;
            keyDto.codeVerifier = command.codeVerifier;
            keyDto.redirectUri = command.redirectUri;

            const std::expected<Port::User::AuthKeyHandlerResult, Port::User::AuthKeyHandlerError> authKeyHandlerResult
                = getAuthKeyHandler().handle(keyDto);
            if (!authKeyHandlerResult)
            {
                return std::unexpected(authKeyHandlerResult.error());
            }

            /** Check on user exsistance */
            if (const auto user = getUserStorage().findByEmail(authKeyHandlerResult->email))
            {
                return AuthUserViaGoogleResult{
                    .userId = user->getId(),
                    .bAlreadyRegistered = true
                };
            }

            /** Adding new user */
            auto uow = getUserStorage().beginWork();

            Domain::User user;

            user.setEmail(authKeyHandlerResult->email);
            user.setVerificationMethod(Domain::VerificationMethod::Google);

            return getUserStorage()
                .add(user)
                .transform([&]
                {
                    uow.commit();
                    return AuthUserViaGoogleResult{
                        .userId = user.getId(),
                        .bAlreadyRegistered = false
                    };
                })
                .transform_error([&](auto&& error)
                {
                    uow.rollback();
                    return Error::MutStrError(error);
                });
        }

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

        AuthKeyHandlerT& _authKeyHandler;
        AuthKeyHandlerT& getAuthKeyHandler() const
        {
            return _authKeyHandler;
        }
    };

} // namespace Service::User
