#pragma once

#include <memory>

#include "Rest/ControllerResolver.h"
#include "Rest/Middleware/AuthMiddleware.h"

namespace Poco
{
    class Logger;
}
namespace Service
{
    class MessageBus;
}

namespace Rest::Resolver
{
    class PluginOptionsResolver
    {
    public:
        PluginOptionsResolver(Middleware::AuthMiddleware& authMiddleware, Service::MessageBus& messageBus, Poco::Logger& logger);

        bool match(const Poco::URI& uri) const;
        Poco::Net::HTTPRequestHandler* make() const;

    protected:
        Middleware::AuthMiddleware& _authMiddleware;
        Middleware::AuthMiddleware& getAuthMiddleware() const
        {
            return _authMiddleware;
        }

        Service::MessageBus& _messageBus;
        Service::MessageBus& getMessageBus() const
        {
            return _messageBus;
        }

        Poco::Logger& _logger;
        Poco::Logger& getLogger() const
        {
            return _logger;
        }
    };

    static_assert(Rest::ControllerResolverC<PluginOptionsResolver>);
} // namespace Rest::Resolver
