#pragma once

namespace Poco
{
    class URI;
}
namespace Poco::Net
{
    class HTTPRequestHandler;
}

namespace Rest
{
    class ControllerResolver
    {
    public:
        virtual ~ControllerResolver() = default;
        virtual bool match(const Poco::URI& uri) const = 0;
        virtual Poco::Net::HTTPRequestHandler* make() const = 0;
    };
}