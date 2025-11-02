#pragma once
#include "Rest/ControllerResolver.h"

namespace Rest::Resolver
{
    class NotFoundControllerResolver : public ControllerResolver
    {
    public:
        bool match(const Poco::URI& uri) const override;
        Poco::Net::HTTPRequestHandler* make() const override;
    };
}