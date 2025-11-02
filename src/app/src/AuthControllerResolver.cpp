#include "Rest/Resolver/AuthControllerResolver.h"

#include <Poco/URI.h>
#include <ctre.hpp>

#include "Rest/Controller/AuthController.h"

using namespace Rest::Resolver;

AuthControllerResolver::AuthControllerResolver(std::shared_ptr<Service::MessageBus> messageBus,
                                                                 Poco::Logger& logger) :
    _messageBus(std::move(messageBus)), _logger(logger)
{}

AuthControllerResolver::~AuthControllerResolver() = default;

bool AuthControllerResolver::match(const Poco::URI& uri) const
{
    return ctre::match<R"(^/api/v0/auth/(?:register|login|refresh|logout)/?$)">(uri.getPath());
}

Poco::Net::HTTPRequestHandler* AuthControllerResolver::make() const
{
    return new Controller::AuthController(getMessageBus(), getLogger());
}


