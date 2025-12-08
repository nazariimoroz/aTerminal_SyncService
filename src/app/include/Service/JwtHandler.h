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
    struct FailedToVerifyJwtError
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

    struct VerifyJwtResult
    {
        int id = 0;

        std::optional<std::string> newAuthToken;
    };
    struct VerifyJwtCommand
    {
        std::string authToken;
        std::string refreshToken;

        using Result = VerifyJwtResult;
        using Error =
            std::variant<Error::StrError, FailedToVerifyJwtError, RefreshTokenExpiredJwtError>;
    };


    class JwtHandler
    {
        JwtHandler(const std::string& secretKey, Service::MessageBus& messageBus);

    public:
        static JwtHandler make(const std::string& secretKey, Service::MessageBus& messageBus);

        CreateJwtCommand::Result execute(const CreateJwtCommand& command) const;

        std::expected<VerifyJwtCommand::Result, VerifyJwtCommand::Error> execute(
            const VerifyJwtCommand& command) const;

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
