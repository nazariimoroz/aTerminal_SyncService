#pragma once
#include <Poco/Net/HTTPRequestHandler.h>

#include "Defines.h"
#include "rfl/patterns.hpp"

namespace Service
{
    class MessageBus;
}
namespace Poco
{
    class Logger;
}

namespace Rest::Controller
{
    struct RegisterAuthRequestDto
    {
        rfl::Email email;
        Defines::Password password;
    };

    struct LoginAuthRequestDto
    {
        rfl::Email email;
        Defines::Password password;
    };


    class AuthController final : public Poco::Net::HTTPRequestHandler
    {
    public:
        AuthController(Service::MessageBus& messageBus, Poco::Logger& logger);

        void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response) override;

    protected:
        void registerUser(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response);
        void loginUser(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response);

    protected:
        Service::MessageBus& _messageBus;
        Service::MessageBus& getMessageBus() const { return _messageBus; }

        Poco::Logger& _logger;
        Poco::Logger& getLogger() const { return _logger; }
    };

} // namespace Rest::Controller
