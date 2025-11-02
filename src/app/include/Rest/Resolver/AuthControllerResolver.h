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
    class AuthControllerResolver final : public ControllerResolver
    {
    public:
        AuthControllerResolver(std::shared_ptr<Service::MessageBus> messageBus, Poco::Logger& logger);
        ~AuthControllerResolver() override;

        bool match(const Poco::URI& uri) const override;
        Poco::Net::HTTPRequestHandler* make() const override;

    protected:
        std::shared_ptr<Service::MessageBus> _messageBus;
        const std::shared_ptr<Service::MessageBus>& getMessageBus() const { return _messageBus; }

        Poco::Logger& _logger;
        Poco::Logger& getLogger() const { return _logger; }
    };
}