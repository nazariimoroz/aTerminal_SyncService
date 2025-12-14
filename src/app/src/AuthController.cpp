#include "Rest/Controller/AuthController.h"

#include <Poco/Logger.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/StreamCopier.h>
#include <rfl.hpp>
#include <rfl/json.hpp>
#include <sstream>
#include <ctre.hpp>

#include "Infra/InMemoryUserStorage.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Service/JwtHandler.h"
#include "Service/MessageBus.h"
#include "Service/User/AuthUserViaGoogleHandler.h"
#include "Service/User/LoginUserHandler.h"
#include "Service/User/RegisterUserHandler.h"
#include "Util/Cbs.h"

using namespace Rest::Controller;

AuthController::AuthController(Service::MessageBus& messageBus, Poco::Logger& logger) :
    _messageBus(messageBus), _logger(logger)
{}

void AuthController::handleRequest(Poco::Net::HTTPServerRequest& request,
                                                     Poco::Net::HTTPServerResponse& response)
{
    response.setContentType("application/json");

    const auto [_, f] = ctre::match<R"(^/api/v0/auth/(google|refresh)/?$)">(request.getURI());
    const auto m = request.getMethod();

    if (m == "POST")
    {
        if (f == "google")
        {
            authUserViaGoogle(request, response);
            return;
        }
        if (f == "refresh")
        {
            refreshAuth(request, response);
            return;
        }
    }

    response.setStatus(Poco::Net::HTTPResponse::HTTP_METHOD_NOT_ALLOWED);
    response.send() << rfl::json::write(Defines::ErrorDTO("Method Not Allowed"));
}

void AuthController::authUserViaGoogle(Poco::Net::HTTPServerRequest& request,
    Poco::Net::HTTPServerResponse& response)
{
    std::stringstream bodyBuffer;
    Poco::StreamCopier::copyStream(request.stream(), bodyBuffer);
    const std::string rawBody = bodyBuffer.str();

    auto dto = rfl::json::read<AuthUserViaGoogleRequest>(rawBody);
    if (!dto)
    {
        response.setStatus(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
        response.send() << rfl::json::write(Defines::ErrorDTO(dto.error().what()));
        return;
    }

    /** Main auth logic */
    Service::User::AuthUserViaGoogleCommand authUserViaGoogleCommand {};
    authUserViaGoogleCommand.code = dto->code;
    authUserViaGoogleCommand.codeVerifier = dto->codeVerifier;
    authUserViaGoogleCommand.redirectUri = dto->redirectUri;
    const auto registerUserResult = getMessageBus().call(authUserViaGoogleCommand);
    if (!registerUserResult)
    {
        response.setStatus(Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
        getLogger().error("UserController::authUserViaGoogle: %s", registerUserResult.error().errorMessage);
        response.send();
        return;
    }

    /** JWT logic */
    Service::CreateJwtCommand createJwtCommand {};
    createJwtCommand.id = registerUserResult->userId;
    const auto createJwtResult = getMessageBus().call(createJwtCommand);

    /** Response */
    response.setStatus(Poco::Net::HTTPResponse::HTTP_CREATED);
    response.send() << rfl::json::write(AuthUserViaGoogleResponse{
        .authToken = createJwtResult.authToken,
        .refreshToken = createJwtResult.refreshToken,
    });
}

void AuthController::refreshAuth(Poco::Net::HTTPServerRequest& request,
    Poco::Net::HTTPServerResponse& response)
{
    std::stringstream bodyBuffer;
    Poco::StreamCopier::copyStream(request.stream(), bodyBuffer);
    const std::string rawBody = bodyBuffer.str();

    auto dto = rfl::json::read<RefreshAuthRequest>(rawBody);
    if (!dto)
    {
        response.setStatus(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
        response.send() << rfl::json::write(Defines::ErrorDTO(dto.error().what()));
        return;
    }

    /** JWT logic */
    Service::RefreshJwtCommand refreshJwtCommand {};
    refreshJwtCommand.authToken = dto->authToken.value();
    refreshJwtCommand.refreshToken = dto->refreshToken.value();
    const auto verifyJwtResult = getMessageBus().call(refreshJwtCommand);

    if (verifyJwtResult)
    {
        response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
        response.send() << rfl::json::write(RefreshAuthResponse{
            .authToken = verifyJwtResult->newAuthToken,
            .refreshToken = verifyJwtResult->newRefreshToken,
        });
    }
    else
    {
        response.setStatus(Poco::Net::HTTPResponse::HTTP_UNAUTHORIZED);
        response.send();
    }
}
