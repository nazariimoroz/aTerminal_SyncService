#pragma once
#include <Poco/JWT/Signer.h>
#include <expected>
#include <memory>
#include <string>
#include <variant>

#include "Util/Errors.h"
#include "Util/LogicException.h"

namespace Service
{
    class MessageBus;
}

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
        using Error =
            std::variant<Error::StrError, AuthTokenExpiredError>;
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


    class JwtHandler
    {
        JwtHandler(const std::string& secretKey, Service::MessageBus& messageBus);

    public:
        static JwtHandler make(const std::string& secretKey, Service::MessageBus& messageBus);

        CreateJwtCommand::Result execute(const CreateJwtCommand& command) const;

        std::expected<RetrieveIdCommand::Result, RetrieveIdCommand::Error> execute(
            const RetrieveIdCommand& command) const;

        std::expected<RefreshJwtCommand::Result, RefreshJwtCommand::Error> execute(
            const RefreshJwtCommand& command) const;

    protected:
        std::string createAuthToken(int id) const;
        std::string createRefreshToken(int id) const;

    protected:
        Service::MessageBus& _messageBus;
        Service::MessageBus& getMessageBus() const
        {
            return _messageBus;
        }

    private:
        std::unique_ptr<Poco::JWT::Signer> _signer;
        const Poco::JWT::Signer& getSigner() const
        {
            return *_signer;
        }
    };
} // namespace Service
