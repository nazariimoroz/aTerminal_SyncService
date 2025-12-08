#pragma once
#include "Rest/ControllerResolver.h"

namespace Rest::Resolver
{
    class NotFoundControllerResolver
    {
    public:
        bool match(const Poco::URI& uri) const;
        Poco::Net::HTTPRequestHandler* make() const;
    };

    static_assert(Rest::ControllerResolverC<NotFoundControllerResolver>);
}