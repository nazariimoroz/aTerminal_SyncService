#include "Rest/Resolver/NotFoundControllerResolver.h"

#include <stdexcept>

#include "Rest/Controller/NotFoundController.h"

using namespace Rest::Resolver;

bool NotFoundControllerResolver::match(const Poco::URI& uri) const
{
    return true;
}

Poco::Net::HTTPRequestHandler* NotFoundControllerResolver::make() const
{
    return new Controller::NotFoundController();
}
