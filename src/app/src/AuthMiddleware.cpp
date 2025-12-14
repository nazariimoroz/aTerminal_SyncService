#include "Rest/Middleware/AuthMiddleware.h"

#include <Poco/Net/HTTPCookie.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>

#include "Service/JwtHandler.h"
#include "Service/MessageBus.h"

using namespace Rest::Middleware;

AuthMiddleware::AuthMiddleware(Service::MessageBus& messageBus) : _messageBus(messageBus)
{
}

std::expected<AuthMiddlewareResult, AuthFailedError> AuthMiddleware::execute(
    Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response) const
{
    auto authToken = std::invoke(
        [&]() -> std::expected<std::string, AuthFailedError>
        {
            auto token = request.get("Authorization");

            static std::string bearer = "Bearer ";
            if (!Poco::startsWith(token, bearer))
            {
                return std::unexpected(AuthFailedError());
            }

            token = token.erase(bearer.size());

            return token;
        });
    if (!authToken) return std::unexpected(authToken.error());

    const auto verifyResult = std::invoke(
        [&] () -> std::expected<Service::RetrieveIdResult, AuthFailedError>
        {
            Service::RetrieveIdCommand retrieveIdCommand;
            retrieveIdCommand.authToken = std::move(*authToken);
            return getMessageBus().call(retrieveIdCommand)
                .transform_error([](auto&& error){ return AuthFailedError(); });
        });
    if (!verifyResult) return std::unexpected(verifyResult.error());

    return AuthMiddlewareResult{.id = verifyResult->id};
}
