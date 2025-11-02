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

using namespace Rest::Controller;

AuthController::AuthController(std::shared_ptr<Service::MessageBus> messageBus,
                                                 Poco::Logger& logger) :
    _messageBus(std::move(messageBus)), _logger(logger)
{}

void AuthController::handleRequest(Poco::Net::HTTPServerRequest& request,
                                                     Poco::Net::HTTPServerResponse& response)
{
    response.setContentType("application/json");

    const auto [_, f] = ctre::match<R"(^/api/v0/auth/(register|login|refresh|logout)/?$)">(request.getURI());
    const auto m = request.getMethod();

    if (m == "POST")
    {
        if (f == "register")
        {
            registerUser(request, response);
            return;
        }
        else if (f == "login")
        {
            loginUser(request, response);
            return;
        }
        else if (f == "refresh")
        {

        }
        else if (f == "logout")
        {

        }
    }

    response.setStatus(Poco::Net::HTTPResponse::HTTP_METHOD_NOT_ALLOWED);
    response.send() << rfl::json::write(Defines::ErrorDTO("Method Not Allowed"));
}

void AuthController::registerUser(Poco::Net::HTTPServerRequest& request,
    Poco::Net::HTTPServerResponse& response)
try
{
    std::stringstream bodyBuffer;
    Poco::StreamCopier::copyStream(request.stream(), bodyBuffer);
    const std::string rawBody = bodyBuffer.str();

    auto dto = rfl::json::read<RegisterAuthRequestDto>(rawBody);
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
    const auto registerUserResult = getMessageBus()->call(registerUserCommand);

    /** JWT logic */
    Service::CreateJwtCommand createJwtCommand {};
    createJwtCommand.id = registerUserResult.userId;
    const auto createJwtResult = getMessageBus()->call(createJwtCommand);

    response.setStatus(Poco::Net::HTTPResponse::HTTP_CREATED);
    response.send() << rfl::json::write(RegisterAuthResponseDto{ .jwt = createJwtResult.jwt });
}
catch (const Port::User::EmailAlreadyRegisteredException& ex)
{
    response.setStatus(Poco::Net::HTTPResponse::HTTP_CONFLICT);
    response.send() << rfl::json::write(Defines::ErrorDTO(ex.what()));
}
catch (std::exception& ex)
{
    response.setStatus(Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
    getLogger().error("UserController::registerUser: %s", ex.what());
}

void AuthController::loginUser(Poco::Net::HTTPServerRequest& request,
                                                 Poco::Net::HTTPServerResponse& response)
try
{
    std::stringstream bodyBuffer;
    Poco::StreamCopier::copyStream(request.stream(), bodyBuffer);
    const std::string rawBody = bodyBuffer.str();

    auto dto = rfl::json::read<LoginAuthRequestDto>(rawBody);
    if (!dto)
    {
        response.setStatus(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
        response.send() << rfl::json::write(Defines::ErrorDTO(dto.error().what()));
        return;
    }

    /** Main login logic */
    Service::User::LoginUserCommand loginUserCommand;
    loginUserCommand.email = std::move(dto->email.value());
    loginUserCommand.rawPassword = std::move(dto->password.value());
    const auto loginUserResult = getMessageBus()->call(loginUserCommand);

    /** JWT logic */
    Service::CreateJwtCommand createJwtCommand {};
    createJwtCommand.id = loginUserResult.userId;
    const auto createJwtResult = getMessageBus()->call(createJwtCommand);

    response.setStatus(Poco::Net::HTTPResponse::HTTP_CREATED);
    response.send() << rfl::json::write(LoginAuthResponseDto{ .jwt = createJwtResult.jwt });
}
catch (const Service::User::InvalidEmailOrPasswordException& ex)
{
    response.setStatus(Poco::Net::HTTPResponse::HTTP_UNAUTHORIZED);
    response.send() << rfl::json::write(Defines::ErrorDTO(ex.what()));
}
catch (std::exception& ex)
{
    response.setStatus(Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
    getLogger().error("UserController::loginUser: %s", ex.what());
}
