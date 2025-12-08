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

    auto refreshToken = std::invoke(
        [&]() -> std::expected<std::string, AuthFailedError>
        {
            Poco::Net::NameValueCollection cookies;
            request.getCookies(cookies);

            std::string token;
            try
            {
                token = cookies.get("refresh_token");
            }
            catch (Poco::NotFoundException&)
            {
                return std::unexpected(AuthFailedError());
            }

            return token;
        });
    if (!refreshToken) return std::unexpected(authToken.error());

    const auto verifyResult = std::invoke(
        [&] () -> std::expected<Service::VerifyJwtResult, AuthFailedError>
        {
            Service::VerifyJwtCommand verifyCommand;
            verifyCommand.authToken = std::move(*authToken);
            verifyCommand.refreshToken = std::move(*refreshToken);
            return getMessageBus().call(verifyCommand)
                .transform_error([](auto&& error){ return AuthFailedError(); });
        });
    if (!verifyResult) return std::unexpected(verifyResult.error());

    /** Set new auth token */
    if (verifyResult->newAuthToken)
    {
        response.set("Authorization", "Bearer " + *verifyResult->newAuthToken);
    }

    return AuthMiddlewareResult{.id = verifyResult->id};
}
