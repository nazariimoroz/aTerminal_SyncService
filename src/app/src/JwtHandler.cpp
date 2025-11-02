#include "Service/JwtHandler.h"
#include "Poco/JWT/Token.h"
#include "Poco/JWT/Signer.h"
#include "Poco/JWT/JWTException.h"
#include "Poco/JSON/Object.h"
#include "Service/MessageBus.h"

using namespace Service;

POCO_IMPLEMENT_EXCEPTION(FailedToVerifyJwtException, Util::LogicException, "FailedToVerifyJwtException")
POCO_IMPLEMENT_EXCEPTION(RefreshTokenExpiredJwtException, Util::BusinessException, "RefreshTokenExpiredJwtException")

JwtHandler::JwtHandler(const std::string& secretKey, std::shared_ptr<Service::MessageBus> messageBus)
    : _messageBus(std::move(messageBus))
    , _signer(secretKey)
{
}

std::shared_ptr<JwtHandler> JwtHandler::make(const std::string& secretKey, std::shared_ptr<Service::MessageBus> messageBus)
{
    const auto self = std::shared_ptr<JwtHandler>(new JwtHandler(secretKey, std::move(messageBus)));

    self->getMessageBus()->registerHandler<CreateJwtCommand>(self, &JwtHandler::execute);
    self->getMessageBus()->registerHandler<VerifyJwtCommand>(self, &JwtHandler::execute);
    return self;
}

CreateJwtResult JwtHandler::execute(const CreateJwtCommand& command) const
{
    CreateJwtResult result;
    result.authToken = createAuthToken(command.id);
    result.refreshToken = createRefreshToken(command.id);

    return result;
}

VerifyJwtResult JwtHandler::execute(const VerifyJwtCommand& command) const
{
    try
    {
        Poco::JWT::Token token = getSigner().verify(command.authToken);
        if (!token.payload().has("id"))
        {
            throw FailedToVerifyJwtException();
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
                throw FailedToVerifyJwtException();
            }
            int id = token.payload().getValue<int>("id");

            auto authToken = createAuthToken(id);

            return VerifyJwtResult{.id = id, .newAuthToken = authToken};
        }
        catch (Poco::JWT::SignatureVerificationException&)
        {
            throw RefreshTokenExpiredJwtException();
        }
    }
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
    auth_token.setExpiration(Poco::Timestamp() + 7 * 24 * 60 * 60  * Poco::Timestamp::resolution()); // +7d

    return getSigner().sign(auth_token, Poco::JWT::Signer::ALGO_HS256);
}
