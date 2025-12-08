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
    self.getMessageBus().registerHandler<VerifyJwtCommand>(self, &JwtHandler::execute);
    return self;
}

CreateJwtCommand::Result JwtHandler::execute(const CreateJwtCommand& command) const
{
    CreateJwtResult result;
    result.authToken = createAuthToken(command.id);
    result.refreshToken = createRefreshToken(command.id);

    return result;
}

std::expected<VerifyJwtCommand::Result, VerifyJwtCommand::Error> JwtHandler::execute(
    const VerifyJwtCommand& command) const
{
    try
    {
        Poco::JWT::Token token = getSigner().verify(command.authToken);
        if (!token.payload().has("id"))
        {
            return std::unexpected(FailedToVerifyJwtError());
        }

        return VerifyJwtResult{.id = token.payload().getValue<int>("id")};
    }
    catch (Poco::JWT::SignatureVerificationException&)
    {
        try
        {
            Poco::JWT::Token token = getSigner().verify(command.refreshToken);
            if (!token.payload().has("id"))
            {
                return std::unexpected(FailedToVerifyJwtError());
            }
            int id = token.payload().getValue<int>("id");

            auto authToken = createAuthToken(id);

            return VerifyJwtResult{.id = id, .newAuthToken = authToken};
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
