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

    const auto [_, f] = ctre::match<R"(^/api/v0/auth/(google)/?$)">(request.getURI());
    const auto m = request.getMethod();

    if (m == "POST")
    {
        if (f == "google")
        {
            authUserViaGoogle(request, response);
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

    auto dto = rfl::json::read<AuthUserViaGoogleDto>(rawBody);
    if (!dto)
    {
        response.setStatus(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
        response.send() << rfl::json::write(Defines::ErrorDTO(dto.error().what()));
        return;
    }

    /** Main register logic */
    Service::User::RegisterUserCommand registerUserCommand {};
    registerUserCommand.email = std::move(dto->email.value());
    registerUserCommand.rawPassword = std::move(dto->password.value());
    const auto registerUserResult = getMessageBus().call(registerUserCommand);
    if (!registerUserResult)
    {
        std::visit(Utils::Cbs(
            [&](const Port::User::EmailAlreadyRegisteredError& error)
            {
                response.setStatus(Poco::Net::HTTPResponse::HTTP_CONFLICT);
                response.send() << rfl::json::write(Defines::ErrorDTO(std::string(error.errorMessage)));
            },
            [&](const Error::StrError& error)
            {
                response.setStatus(Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
                getLogger().error("UserController::registerUser: %s", error.errorMessage);
                response.send();
            }
            ), registerUserResult.error());
        return;
    }

    /** JWT logic */
    Service::CreateJwtCommand createJwtCommand {};
    createJwtCommand.id = registerUserResult->userId;
    const auto createJwtResult = getMessageBus().call(createJwtCommand);

    /** Response */
    response.set("Authorization", "Bearer " + createJwtResult.authToken);

    Poco::Net::HTTPCookie cookie("refresh_token", createJwtResult.refreshToken);
    cookie.setHttpOnly(true);
    cookie.setSecure(true);
    cookie.setPath("/");
    cookie.setMaxAge(7 * 24 * 60 * 60);
    cookie.setSameSite(Poco::Net::HTTPCookie::SAME_SITE_STRICT);
    response.addCookie(cookie);

    response.setStatus(Poco::Net::HTTPResponse::HTTP_CREATED);
    response.send();
}

void AuthController::loginUser(Poco::Net::HTTPServerRequest& request,
                                                 Poco::Net::HTTPServerResponse& response)
{
    // std::stringstream bodyBuffer;
    // Poco::StreamCopier::copyStream(request.stream(), bodyBuffer);
    // const std::string rawBody = bodyBuffer.str();
// 
    // auto dto = rfl::json::read<AuthUserViaGoogleDto>(rawBody);
    // if (!dto)
    // {
    //     response.setStatus(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
    //     response.send() << rfl::json::write(Defines::ErrorDTO(dto.error().what()));
    //     return;
    // }
// 
    // /** Main login logic */
    // Service::User::LoginUserCommand loginUserCommand;
    // loginUserCommand.email = std::move(dto->email.value());
    // loginUserCommand.rawPassword = std::move(dto->password.value());
    // const auto loginUserResult = getMessageBus().call(loginUserCommand);
    // if (!loginUserResult)
    // {
    //     response.setStatus(Poco::Net::HTTPResponse::HTTP_UNAUTHORIZED);
    //     response.send() << rfl::json::write(Defines::ErrorDTO(std::string(loginUserResult.error().errorMessage)));
    //     return;
    // }
// 
    // /** JWT logic */
    // Service::CreateJwtCommand createJwtCommand {};
    // createJwtCommand.id = loginUserResult->userId;
    // const auto createJwtResult = getMessageBus().call(createJwtCommand);
// 
    // /** Response */
    // response.set("Authorization", "Bearer " + createJwtResult.authToken);
// 
    // Poco::Net::HTTPCookie cookie("refresh_token", createJwtResult.refreshToken);
    // cookie.setHttpOnly(true);
    // cookie.setSecure(true);
    // cookie.setPath("/");
    // cookie.setMaxAge(7 * 24 * 60 * 60);
    // cookie.setSameSite(Poco::Net::HTTPCookie::SAME_SITE_STRICT);
    // response.addCookie(cookie);
// 
    // response.setStatus(Poco::Net::HTTPResponse::HTTP_CREATED);
    // response.send();
}
