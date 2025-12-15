#pragma once

#include <Poco/JWT/JWTException.h>
#include <Poco/JWT/Signer.h>
#include <Poco/JWT/Token.h>
#include <Poco/Timestamp.h>

#include <expected>
#include <memory>
#include <optional>
#include <string>
#include <variant>

#include "Port/User/UserStorage.h"
#include "Service/MessageBus.h"
#include "Util/Errors.h"
#include "Util/LogicException.h"

namespace Service
{
    struct AuthTokenExpiredError
    {
    };
    struct FailedToRefreshJwtError
    {
    };
    struct RefreshTokenExpiredJwtError
    {
    };

    struct CreateJwtResult
    {
        std::string authToken;
        std::string refreshToken;
    };
    struct CreateJwtCommand
    {
        int id = 0;

        using Result = CreateJwtResult;
        using Error = void;
    };

    struct RetrieveIdResult
    {
        int id;
    };
    struct RetrieveIdCommand
    {
        std::string authToken;

        using Result = RetrieveIdResult;
        using Error = std::variant<Error::StrError, AuthTokenExpiredError>;
    };

    struct RefreshJwtResult
    {
        std::string newAuthToken;
        std::string newRefreshToken;
    };
    struct RefreshJwtCommand
    {
        std::string authToken;
        std::string refreshToken;

        using Result = RefreshJwtResult;
        using Error =
            std::variant<Error::StrError, FailedToRefreshJwtError, RefreshTokenExpiredJwtError>;
    };

    template <Port::User::UserStorageC UserStorageT>
    class JwtHandler;

    template <Port::User::UserStorageC UserStorageT>
    std::shared_ptr<JwtHandler<UserStorageT>> makeJwtHandler(Service::MessageBus& messageBus,
                                                             UserStorageT& userStorage,
                                                             const std::string& secretKey);

    template <Port::User::UserStorageC UserStorageT>
    class JwtHandler
    {
        JwtHandler(const std::string& secretKey, Service::MessageBus& messageBus, UserStorageT& userStorage) :
            _messageBus(messageBus),
            _userStorage(userStorage),
            _signer(std::make_unique<Poco::JWT::Signer>(secretKey))
        {
        }

        template <Port::User::UserStorageC UST>
        friend std::shared_ptr<JwtHandler<UST>> makeJwtHandler(Service::MessageBus&,
                                                               UST&,
                                                               const std::string&);

    public:
        JwtHandler(JwtHandler&&) = default;

        CreateJwtCommand::Result execute(const CreateJwtCommand& command) const
        {
            CreateJwtResult result;
            result.authToken = createAuthToken(command.id);
            result.refreshToken = createRefreshToken(command.id);
            return result;
        }

        std::expected<RetrieveIdCommand::Result, RetrieveIdCommand::Error> execute(
            const RetrieveIdCommand& command) const
        {
            try
            {
                Poco::JWT::Token token = getSigner().verify(command.authToken);

                if (!token.payload().has("id"))
                {
                    return std::unexpected(Error::StrError("Failed to retrieve id from token"));
                }

                const int id = token.payload().getValue<int>("id");

                if (!getUserStorage().findById(id))
                {
                    return std::unexpected(Error::StrError("User not found"));
                }

                return RetrieveIdResult{.id = id};
            }
            catch (const Poco::JWT::SignatureVerificationException&)
            {
                return std::unexpected(AuthTokenExpiredError{});
            }
            catch (const std::exception& e)
            {
                return std::unexpected(Error::StrError(e.what()));
            }
        }

        std::expected<RefreshJwtCommand::Result, RefreshJwtCommand::Error> execute(
            const RefreshJwtCommand& command) const
        {
            try
            {
                Poco::JWT::Token token = getSigner().verify(command.authToken);

                if (!token.payload().has("id"))
                {
                    return std::unexpected(FailedToRefreshJwtError{});
                }

                const int id = token.payload().getValue<int>("id");

                if (!getUserStorage().findById(id))
                {
                    return std::unexpected(FailedToRefreshJwtError{});
                }

                return RefreshJwtResult{.newAuthToken = command.authToken,
                                        .newRefreshToken = command.refreshToken};
            }
            catch (const Poco::JWT::SignatureVerificationException&)
            {
                try
                {
                    Poco::JWT::Token token = getSigner().verify(command.refreshToken);

                    if (!token.payload().has("id"))
                    {
                        return std::unexpected(FailedToRefreshJwtError{});
                    }

                    const int id = token.payload().getValue<int>("id");

                    if (!getUserStorage().findById(id))
                    {
                        return std::unexpected(FailedToRefreshJwtError{});
                    }

                    const auto newAuthToken = createAuthToken(id);

                    return RefreshJwtResult{.newAuthToken = newAuthToken,
                                            .newRefreshToken = command.refreshToken};
                }
                catch (const Poco::JWT::SignatureVerificationException&)
                {
                    return std::unexpected(RefreshTokenExpiredJwtError{});
                }
                catch (const std::exception& e)
                {
                    return std::unexpected(Error::StrError(e.what()));
                }
            }
            catch (const std::exception& e)
            {
                return std::unexpected(Error::StrError(e.what()));
            }
        }

    protected:
        std::string createAuthToken(int id) const
        {
            Poco::JWT::Token t;
            t.setType("JWT");
            t.payload().set("id", id);
            t.setIssuedAt(Poco::Timestamp());
            t.setExpiration(Poco::Timestamp() + 15 * 60 * Poco::Timestamp::resolution());
            return getSigner().sign(t, Poco::JWT::Signer::ALGO_HS256);
        }

        std::string createRefreshToken(int id) const
        {
            Poco::JWT::Token t;
            t.setType("JWT");
            t.payload().set("id", id);
            t.setIssuedAt(Poco::Timestamp());
            t.setExpiration(Poco::Timestamp() + 7 * 24 * 60 * 60 * Poco::Timestamp::resolution());
            return getSigner().sign(t, Poco::JWT::Signer::ALGO_HS256);
        }

    protected:
        Service::MessageBus& _messageBus;
        Service::MessageBus& getMessageBus() const { return _messageBus; }

        UserStorageT& _userStorage;
        UserStorageT& getUserStorage() const { return _userStorage; }

    private:
        std::unique_ptr<Poco::JWT::Signer> _signer;
        const Poco::JWT::Signer& getSigner() const { return *_signer; }
    };

    template <Port::User::UserStorageC UserStorageT>
    inline std::shared_ptr<JwtHandler<UserStorageT>> makeJwtHandler(Service::MessageBus& messageBus,
                                                                    UserStorageT& userStorage,
                                                                    const std::string& secretKey)
    {
        using JwtHandlerT = JwtHandler<UserStorageT>;

        auto self =
            std::shared_ptr<JwtHandlerT>(new JwtHandlerT(secretKey, messageBus, userStorage));

        messageBus.registerHandler<CreateJwtCommand>(self, &JwtHandlerT::execute);
        messageBus.registerHandler<RetrieveIdCommand>(self, &JwtHandlerT::execute);
        messageBus.registerHandler<RefreshJwtCommand>(self, &JwtHandlerT::execute);

        return self;
    }

} // namespace Service
