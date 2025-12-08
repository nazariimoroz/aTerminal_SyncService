#pragma once

#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <functional>

#include "Resolver/NotFoundControllerResolver.h"

namespace Service
{
    class MessageBus;
}
namespace Poco
{
    class Logger;
}

namespace Rest
{
    template <ControllerResolverC NotfoundControllerResolverT,
              ControllerResolverC... ControllerResolverTs>
    class RequestRouter final : public Poco::Net::HTTPRequestHandlerFactory
    {
    public:
        RequestRouter(const std::tuple<ControllerResolverTs...>& controllerResolvers,
                      NotfoundControllerResolverT&& notFoundResolver);

        Poco::Net::HTTPRequestHandler* createRequestHandler(
            const Poco::Net::HTTPServerRequest& request) override;

    private:
        const std::tuple<ControllerResolverTs...>& _controllerResolvers;
        const std::tuple<ControllerResolverTs...>& getControllerResolvers() const
        {
            return _controllerResolvers;
        }

        NotfoundControllerResolverT& _notFoundControllerResolver;
        NotfoundControllerResolverT& getNotFoundControllerResolver() const
        {
            return _notFoundControllerResolver;
        }
    };
} // namespace Rest

#include "RequestRouter.inl"
