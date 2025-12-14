#include "Infra/GoogleAuthKeyHandler.h"

#include <Poco/Logger.h>
#include <Poco/Net/AcceptCertificateHandler.h>
#include <Poco/Net/Context.h>
#include <Poco/Net/HTMLForm.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/Net/SSLManager.h>
#include <Poco/URI.h>

#include "rfl/json/Parser.hpp"
#include "rfl/json/load.hpp"

Infra::GoogleAuthKeyHandler::GoogleAuthKeyHandler(std::string clientId, std::string clientSecret, Poco::Logger& logger)
    : _clientId(std::move(clientId))
    , _clientSecret(std::move(clientSecret))
    , _logger(logger)
{
}

std::expected<Port::User::AuthKeyHandlerResult, Port::User::AuthKeyHandlerError> Infra::
    GoogleAuthKeyHandler::handle(const KeyDtoT& dto) const
{
    Poco::Net::initializeSSL();

    auto* certHandler = new Poco::Net::AcceptCertificateHandler(false);
    auto* ctx = new Poco::Net::Context(Poco::Net::Context::CLIENT_USE, "", "", "",
                                       Poco::Net::Context::VERIFY_RELAXED, 9, true, "ALL");
    Poco::Net::SSLManager::instance().initializeClient(nullptr, certHandler, ctx);

    Poco::URI uri("https://oauth2.googleapis.com/token");

    Poco::Net::HTTPSClientSession session(uri.getHost(), uri.getPort());
    std::string path = uri.getPathEtc();
    if (path.empty())
        path = "/";

    Poco::Net::HTTPRequest req(Poco::Net::HTTPRequest::HTTP_POST, path,
                               Poco::Net::HTTPMessage::HTTP_1_1);
    req.setKeepAlive(false);
    req.set("User-Agent", "MyApp/1.0");

    std::string codeDecoded;
    Poco::URI::decode(dto.code, codeDecoded);

    Poco::Net::HTMLForm form;
    form.setEncoding(Poco::Net::HTMLForm::ENCODING_URL);
    form.set("grant_type", "authorization_code");
    form.set("code", codeDecoded);
    form.set("client_id", getClientId());
    form.set("client_secret", getClientSecret());
    form.set("redirect_uri", dto.redirectUri.value());
    form.set("code_verifier", dto.codeVerifier.value());

    form.prepareSubmit(req);

    {
        std::ostream& os = session.sendRequest(req);
        form.write(os);
    }

    Poco::Net::HTTPResponse res;
    std::ostringstream body;
    {
        std::istream& is = session.receiveResponse(res);
        Poco::StreamCopier::copyStream(is, body);
    }

    if (res.getStatus() < 200 || res.getStatus() >= 300)
    {
        return std::unexpected(Error::MutStrError(
            "Google token endpoint error " + std::to_string(res.getStatus()) + ": " + body.str()));
    }

    struct GoogleTokenResponse
    {
        std::string access_token;
        std::string id_token;
        std::string refresh_token;
        int expires_in = 0;
        std::string token_type;
    };

    auto response = rfl::json::read<GoogleTokenResponse>(body.str());
    if (!response)
    {
        return std::unexpected(
            Error::MutStrError("Google token endpoint error: " + response.error().what()));
    }

    Poco::Net::uninitializeSSL();

    return fetchResult(response->access_token);
}

std::expected<Port::User::AuthKeyHandlerResult, Port::User::AuthKeyHandlerError> Infra::
    GoogleAuthKeyHandler::fetchResult(const std::string& accessToken) const
{
    Poco::URI uri("https://openidconnect.googleapis.com/v1/userinfo");

    Poco::Net::HTTPSClientSession session(uri.getHost(), uri.getPort());
    std::string path = uri.getPathEtc();
    if (path.empty())
        path = "/";

    Poco::Net::HTTPRequest req(Poco::Net::HTTPRequest::HTTP_GET, path,
                               Poco::Net::HTTPMessage::HTTP_1_1);
    req.setKeepAlive(false);
    req.set("User-Agent", "MyApp/1.0");
    req.set("Authorization", "Bearer " + accessToken);

    session.sendRequest(req);

    Poco::Net::HTTPResponse res;
    std::ostringstream body;
    {
        std::istream& is = session.receiveResponse(res);
        Poco::StreamCopier::copyStream(is, body);
    }

    if (res.getStatus() < 200 || res.getStatus() >= 300)
    {
        return std::unexpected(Error::MutStrError("userinfo error " + std::to_string(res.getStatus()) + ": " +
                                 body.str()));
    }

    struct GoogleUserInfo
    {
        std::string sub;
        std::string name;
        rfl::Rename<"given_name", std::string> givenName;
        rfl::Rename<"family_name", std::string> familyName;
        std::string picture;
        std::string email;
        bool email_verified = false;
    };

    const auto googleUserInfo = rfl::json::read<GoogleUserInfo>(body.str());
    if (!googleUserInfo)
    {
        return std::unexpected(Error::MutStrError("userinfo error: " + googleUserInfo.error().what()));
    }

    Port::User::AuthKeyHandlerResult result;
    result.email = googleUserInfo->email;

    return result;
}
