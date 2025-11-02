#pragma once
#include <Poco/JWT/Signer.h>
#include <memory>
#include <string>

#include "Util/BusinessException.h"
#include "Util/LogicException.h"

namespace Service
{
    class MessageBus;
}

namespace Service
{
    POCO_DECLARE_EXCEPTION(, FailedToVerifyJwtException, Util::LogicException)
    POCO_DECLARE_EXCEPTION(, RefreshTokenExpiredJwtException, Util::BusinessException)

    struct CreateJwtResult
    {
        std::string authToken;
        std::string refreshToken;
    };
    struct CreateJwtCommand
    {
        int id;

        using Result = CreateJwtResult;
    };

    struct VerifyJwtResult
    {
        int id;

        std::optional<std::string> newAuthToken;
    };
    struct VerifyJwtCommand
    {
        std::string authToken;
        std::string refreshToken;

        using Result = VerifyJwtResult;
    };


    class JwtHandler
    {
        JwtHandler(const std::string& secretKey, std::shared_ptr<Service::MessageBus> messageBus);

    public:
        static std::shared_ptr<JwtHandler> make(const std::string& secretKey,
                                                std::shared_ptr<Service::MessageBus> messageBus);

        CreateJwtResult execute(const CreateJwtCommand& command) const;
        VerifyJwtResult execute(const VerifyJwtCommand& command) const;

    protected:
        std::string createAuthToken(int id) const;
        std::string createRefreshToken(int id) const;

    protected:
        std::shared_ptr<Service::MessageBus> _messageBus;
        const std::shared_ptr<Service::MessageBus>& getMessageBus() const
        {
            return _messageBus;
        }

    private:
        Poco::JWT::Signer _signer;
        const Poco::JWT::Signer& getSigner() const { return _signer; }
    };
}


