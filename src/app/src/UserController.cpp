#include "Rest/Controller/UserController.h"

#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/StreamCopier.h>
#include <rfl.hpp>
#include <rfl/json.hpp>
#include <sstream>
#include "Poco/Net/HTTPServerResponse.h"

void Rest::Controller::UserController::handleRequest(Poco::Net::HTTPServerRequest& request,
                                                     Poco::Net::HTTPServerResponse& response)
{
    response.setContentType("application/json");

    const auto m = request.getMethod();
    if (m == "POST")
        registerUser(request, response);
    else
    {
        response.setStatus(Poco::Net::HTTPResponse::HTTP_METHOD_NOT_ALLOWED);
        response.send() << rfl::json::write(Defines::ErrorDTO("Method Not Allowed"));
    }
}

void Rest::Controller::UserController::registerUser(Poco::Net::HTTPServerRequest& request,
                                                    Poco::Net::HTTPServerResponse& response)
{
    std::stringstream bodyBuffer;
    Poco::StreamCopier::copyStream(request.stream(), bodyBuffer);
    const std::string rawBody = bodyBuffer.str();

    auto dto = rfl::json::read<RegisterUserDto>(rawBody);
    if (!dto)
    {
        response.setStatus(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
        response.send() << rfl::json::write(Defines::ErrorDTO(dto.error().what()));
        return;
    }

    response.setStatus(Poco::Net::HTTPResponse::HTTP_CREATED);
    response.send() << rfl::json::write(dto);
}
