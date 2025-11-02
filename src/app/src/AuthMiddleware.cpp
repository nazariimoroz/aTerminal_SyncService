#include "Rest/Middleware/AuthMiddleware.h"

#include <Poco/Net/HTTPCookie.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>

#include "Service/JwtHandler.h"
#include "Service/MessageBus.h"

using namespace Rest::Middleware;

POCO_IMPLEMENT_EXCEPTION(AuthFailedException, Util::BusinessException, "AuthFailedException")

AuthMiddleware::AuthMiddleware(std::shared_ptr<Service::MessageBus> messageBus) : _messageBus(std::move(messageBus))
{
}

AuthMiddlewareResult AuthMiddleware::execute(Poco::Net::HTTPServerRequest& request,
                                             Poco::Net::HTTPServerResponse& response) const
{
    auto authToken = std::invoke(
        [&]
        {
            auto token = request.get("Authorization");

            static std::string bearer = "Bearer ";
            if (!Poco::startsWith(token, bearer))
                throw AuthFailedException();

            token = token.erase(bearer.size());

            return token;
        });

    auto refreshToken = std::invoke(
        [&]
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
                throw AuthFailedException();
            }

            return token;
        });

    const auto verifyResult = std::invoke(
        [&]
        {
            try
            {
                Service::VerifyJwtCommand verifyCommand;
                verifyCommand.authToken = std::move(authToken);
                verifyCommand.refreshToken = std::move(refreshToken);
                return getMessageBus()->call(verifyCommand);
            }
            catch (Service::RefreshTokenExpiredJwtException&)
            {
                throw AuthFailedException();
            }
        });

    /** Set new auth token */
    if (verifyResult.newAuthToken)
    {
        response.set("Authorization", "Bearer " + *verifyResult.newAuthToken);
    }

    return AuthMiddlewareResult{.id = verifyResult.id};
}
