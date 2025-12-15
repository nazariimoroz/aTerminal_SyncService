#pragma once
#include <Poco/Net/HTTPRequestHandler.h>

#include "rfl/Rename.hpp"


namespace Rest::Middleware
{
    class AuthMiddleware;
}

namespace Service
{
    class MessageBus;
}

namespace Poco
{
    namespace Net
    {
        class HTTPServerResponse;
        class HTTPServerRequest;
    }

    class Logger;
}

namespace Rest::Controller
{
    struct PluginOptionsGetRequest
    {
        rfl::Rename<"plugin_name", std::string> pluginName;
    };

    struct PluginOptionsGetResponse
    {
        rfl::Rename<"plugin_name", std::string> pluginName;
        rfl::Rename<"is_enabled", bool> isEnabled;
        rfl::Rename<"is_favorite", bool> isFavorite;
        rfl::Rename<"fields", std::unordered_map<std::string, std::string>> fields;
    };

    struct PluginOptionsPostRequest
    {
        rfl::Rename<"plugin_name", std::string> pluginName;
        rfl::Rename<"is_enabled", bool> isEnabled;
        rfl::Rename<"is_favorite", bool> isFavorite;
        rfl::Rename<"fields", std::unordered_map<std::string, std::string>> fields;
    };

    struct PluginOptionsPostResponse
    {
    };


    class PluginOptionsController final : public Poco::Net::HTTPRequestHandler
    {
    public:
        PluginOptionsController(Middleware::AuthMiddleware& authMiddleware, Service::MessageBus& messageBus,
                                Poco::Logger& logger);

        void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response) override;

    protected:
        void handleGetRequest(int userId, Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response);
        void handlePostRequest(int userId, Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response);

    protected:
        Middleware::AuthMiddleware& _authMiddleware;
        Middleware::AuthMiddleware& getAuthMiddleware() const { return _authMiddleware; };

        Service::MessageBus& _messageBus;
        Service::MessageBus& getMessageBus() const { return _messageBus; }

        Poco::Logger& _logger;
        Poco::Logger& getLogger() const { return _logger; }
    };
}
