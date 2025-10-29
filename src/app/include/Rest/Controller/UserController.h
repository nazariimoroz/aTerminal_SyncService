#pragma once
#include <Poco/Net/HTTPRequestHandler.h>

#include "Defines.h"
#include "rfl/patterns.hpp"

namespace Rest::Controller
{
    struct RegisterUserDto
    {
        rfl::Email email;
        Defines::Password password;
    };

    class UserController : public Poco::Net::HTTPRequestHandler
    {
    public:
        void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response) override;

    protected:
        void registerUser(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response);
    };
} // namespace Rest::Controller
