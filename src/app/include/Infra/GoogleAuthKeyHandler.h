#pragma once
#include <rfl/Rename.hpp>
#include "Port/User/AuthKeyHandler.h"

namespace Poco
{
    class Logger;
}
namespace Infra
{
    struct GoogleAuthKeyDto
    {
        std::string code;
        rfl::Rename<"code_verifier", std::string> codeVerifier;
        rfl::Rename<"redirect_uri", std::string> redirectUri;
    };

    class GoogleAuthKeyHandler
    {
    public:
        using KeyDtoT = GoogleAuthKeyDto;

        GoogleAuthKeyHandler(std::string clientId, std::string clientSecret, Poco::Logger& logger);

        std::expected<Port::User::AuthKeyHandlerResult, Port::User::AuthKeyHandlerError> handle(
            const KeyDtoT& dto) const;

    private:
        std::expected<Port::User::AuthKeyHandlerResult, Port::User::AuthKeyHandlerError> fetchResult(const std::string& accessToken) const;

    private:
        std::string _clientId;
        const std::string& getClientId() const { return _clientId; }

        std::string _clientSecret;
        const std::string& getClientSecret() const { return _clientSecret; }

        Poco::Logger& _logger;
        Poco::Logger& getLogger() const { return _logger; }
    };

    static_assert(Port::User::AuthKeyHandlerC<GoogleAuthKeyHandler>);
} // namespace Infra
