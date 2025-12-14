#include "Rest/Resolver/AuthControllerResolver.h"

#include <Poco/URI.h>
#include <ctre.hpp>

#include "Rest/Controller/AuthController.h"

using namespace Rest::Resolver;

AuthControllerResolver::AuthControllerResolver(Service::MessageBus& messageBus,
                                                                 Poco::Logger& logger) :
    _messageBus(messageBus), _logger(logger)
{}

bool AuthControllerResolver::match(const Poco::URI& uri) const
{
    return ctre::match<R"(^/api/v0/auth/(?:google|refresh)/?$)">(uri.getPath());
}

Poco::Net::HTTPRequestHandler* AuthControllerResolver::make() const
{
    return new Controller::AuthController(getMessageBus(), getLogger());
}


