#include "Service/JwtHandler.h"
#include "Poco/JSON/Object.h"
#include "Poco/JWT/JWTException.h"
#include "Poco/JWT/Signer.h"
#include "Poco/JWT/Token.h"
#include "Service/MessageBus.h"

using namespace Service;

JwtHandler::JwtHandler(const std::string& secretKey, Service::MessageBus& messageBus) :
    _messageBus(messageBus), _signer(std::make_unique<Poco::JWT::Signer>(secretKey))
{
}
JwtHandler JwtHandler::make(const std::string& secretKey, Service::MessageBus& messageBus)
{
    auto self = JwtHandler(secretKey, messageBus);

    self.getMessageBus().registerHandler<CreateJwtCommand>(self, &JwtHandler::execute);
    self.getMessageBus().registerHandler<RetrieveIdCommand>(self, &JwtHandler::execute);
    self.getMessageBus().registerHandler<RefreshJwtCommand>(self, &JwtHandler::execute);
    return self;
}

CreateJwtCommand::Result JwtHandler::execute(const CreateJwtCommand& command) const
{
    CreateJwtResult result;
    result.authToken = createAuthToken(command.id);
    result.refreshToken = createRefreshToken(command.id);

    return result;
}

std::expected<RetrieveIdCommand::Result, RetrieveIdCommand::Error> JwtHandler::execute(
    const RetrieveIdCommand& command) const
{
    try
    {
        Poco::JWT::Token token = getSigner().verify(command.authToken);
        if (!token.payload().has("id"))
        {
            return std::unexpected(Error::StrError("Failed to retrieve id from token"));
        }
        int id = token.payload().getValue<int>("id");

        return RetrieveIdResult{
            .id = id
        };
    }
    catch (Poco::JWT::SignatureVerificationException&)
    {
        return std::unexpected(AuthTokenExpiredError());
    }
}

std::expected<RefreshJwtCommand::Result, RefreshJwtCommand::Error> JwtHandler::execute(
    const RefreshJwtCommand& command) const
{
    try
    {
        Poco::JWT::Token token = getSigner().verify(command.authToken);
        if (!token.payload().has("id"))
        {
            return std::unexpected(FailedToRefreshJwtError());
        }

        return RefreshJwtResult{
            .newAuthToken = command.authToken,
            .newRefreshToken = command.refreshToken
        };
    }
    catch (Poco::JWT::SignatureVerificationException&)
    {
        try
        {
            Poco::JWT::Token token = getSigner().verify(command.refreshToken);
            if (!token.payload().has("id"))
            {
                return std::unexpected(FailedToRefreshJwtError());
            }
            int id = token.payload().getValue<int>("id");

            auto authToken = createAuthToken(id);

            return RefreshJwtResult{
                .newAuthToken = authToken,
                .newRefreshToken = command.refreshToken
            };
        }
        catch (Poco::JWT::SignatureVerificationException&)
        {
            return std::unexpected(RefreshTokenExpiredJwtError());
        }
    }

    return std::unexpected(Error::StrError());
}

std::string JwtHandler::createAuthToken(int id) const
{
    Poco::JWT::Token auth_token;
    auth_token.setType("JWT");
    // token.setSubject("");
    auth_token.payload().set("id", id);
    auth_token.setIssuedAt(Poco::Timestamp());
    auth_token.setExpiration(Poco::Timestamp() + 15 * 60 * Poco::Timestamp::resolution()); // +15m

    return getSigner().sign(auth_token, Poco::JWT::Signer::ALGO_HS256);
}

std::string JwtHandler::createRefreshToken(int id) const
{
    Poco::JWT::Token auth_token;
    auth_token.setType("JWT");
    // token.setSubject("");
    auth_token.payload().set("id", id);
    auth_token.setIssuedAt(Poco::Timestamp());
    auth_token.setExpiration(Poco::Timestamp() +
                             7 * 24 * 60 * 60 * Poco::Timestamp::resolution()); // +7d

    return getSigner().sign(auth_token, Poco::JWT::Signer::ALGO_HS256);
}
