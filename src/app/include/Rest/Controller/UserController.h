#pragma once
#include <Poco/Net/HTTPRequestHandler.h>

namespace Api
{
    struct RegisterUserDto
    {
        std::string email;
        std::string password;
    };

    class UserController : public Poco::Net::HTTPRequestHandler {
    public:
        void handleRequest(Poco::Net::HTTPServerRequest& request,
            Poco::Net::HTTPServerResponse& response) override;

    protected:
        void registerUser(Poco::Net::HTTPServerRequest& request,
            Poco::Net::HTTPServerResponse& response);
    };
}
