#pragma once
#include <Poco/Net/HTTPRequestHandler.h>

#include "Defines.h"
#include "rfl/Rename.hpp"
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
    struct AuthUserViaGoogleRequest
    {
        std::string code;
        rfl::Rename<"code_verifier", std::string> codeVerifier;
        rfl::Rename<"redirect_uri", std::string> redirectUri;
    };

    struct AuthUserViaGoogleResponse
    {
        rfl::Rename<"auth_token", std::string> authToken;
        rfl::Rename<"refresh_token", std::string> refreshToken;
    };


    struct RefreshAuthRequest
    {
        rfl::Rename<"auth_token", std::string> authToken;
        rfl::Rename<"refresh_token", std::string> refreshToken;
    };

    struct RefreshAuthResponse
    {
        rfl::Rename<"auth_token", std::string> authToken;
        rfl::Rename<"refresh_token", std::string> refreshToken;
    };


    class AuthController final : public Poco::Net::HTTPRequestHandler
    {
    public:
        AuthController(Service::MessageBus& messageBus, Poco::Logger& logger);

        void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response) override;

    protected:
        void authUserViaGoogle(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response);
        void refreshAuth(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response);

    protected:
        Service::MessageBus& _messageBus;
        Service::MessageBus& getMessageBus() const { return _messageBus; }

        Poco::Logger& _logger;
        Poco::Logger& getLogger() const { return _logger; }
    };

} // namespace Rest::Controller
