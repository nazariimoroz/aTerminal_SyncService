#pragma once
#include <memory>


#include "Rest/ControllerResolver.h"

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
    class AuthControllerResolver
    {
    public:
        AuthControllerResolver(Service::MessageBus& messageBus, Poco::Logger& logger);

        bool match(const Poco::URI& uri) const;
        Poco::Net::HTTPRequestHandler* make() const;

    protected:
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

    static_assert(Rest::ControllerResolverC<AuthControllerResolver>);
} // namespace Rest::Resolver
