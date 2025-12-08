#pragma once
#include <concepts>

namespace Poco
{
    namespace Net
    {
        class HTTPRequestHandler;
    }
    class URI;
}

namespace Rest
{
    template<class T>
    concept ControllerResolverC = requires (T t, const Poco::URI& uri)
    {
        { t.match(uri) } -> std::same_as<bool>;
        { t.make() } -> std::same_as<Poco::Net::HTTPRequestHandler*>;
    };
}