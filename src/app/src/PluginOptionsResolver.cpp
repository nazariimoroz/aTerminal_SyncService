#include "Rest/Resolver/PluginOptionsResolver.h"

#include <Poco/URI.h>

#include "Rest/Controller/AuthController.h"
#include "Rest/Controller/PluginOptionsController.h"

namespace Rest::Resolver
{
    PluginOptionsResolver::PluginOptionsResolver(Middleware::AuthMiddleware& authMiddleware,
                                                 Service::MessageBus& messageBus,
                                                 Poco::Logger& logger) :
        _authMiddleware(authMiddleware), _messageBus(messageBus), _logger(logger)
    {
    }

    bool PluginOptionsResolver::match(const Poco::URI& uri) const
    {
        return ctre::match<R"(^/api/v0/plugin/options(?:\?.*)?$)">(uri.getPath());
    }

    Poco::Net::HTTPRequestHandler* PluginOptionsResolver::make() const
    {
        return new Rest::Controller::PluginOptionsController(getAuthMiddleware(), getMessageBus(), getLogger());
    }
} // namespace Rest::Resolver
