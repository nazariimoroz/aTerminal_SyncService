#include "Service/JwtHandler.h"
#include "Poco/JWT/Token.h"
#include "Poco/JWT/Signer.h"
#include "Poco/JWT/JWTException.h"
#include "Poco/JSON/Object.h"
#include "Service/MessageBus.h"

using namespace Service;

POCO_IMPLEMENT_EXCEPTION(FailedToVerifyJwtException, Util::BusinessException, "FailedToVerifyJwtException")

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
    Poco::JWT::Token token;
    token.setType("JWT");
    // token.setSubject("");
    token.payload().set("id", command.id);
    token.setIssuedAt(Poco::Timestamp());
    token.setExpiration(Poco::Timestamp() + 60 * 60 * Poco::Timestamp::resolution()); // +1h

    auto jwt = getSigner().sign(token, Poco::JWT::Signer::ALGO_HS256);

    return CreateJwtResult{.jwt = std::move(jwt)};
}

VerifyJwtResult JwtHandler::execute(const VerifyJwtCommand& command) const
try
{
    Poco::JWT::Token token = getSigner().verify(command.jwt);
    if (!token.payload().has("id"))
    {
        throw FailedToVerifyJwtException();
    }

    int id = token.payload().getValue<int>("id");
    return VerifyJwtResult{.id = id};
}
catch (Poco::JWT::SignatureVerificationException&)
{
    throw FailedToVerifyJwtException();
}