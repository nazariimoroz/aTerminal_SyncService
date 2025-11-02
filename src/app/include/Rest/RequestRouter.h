#pragma once

#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <functional>

#include "Resolver/NotFoundControllerResolver.h"

namespace Rest
{
    class ControllerResolver;
}
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
    struct Route
    {
        std::function<bool(const std::string&)> validator;
        std::function<Poco::Net::HTTPRequestHandler*()> controllerMaker;
    };

    class RequestRouter final : public Poco::Net::HTTPRequestHandlerFactory
    {
    public:
        explicit RequestRouter(std::vector<std::shared_ptr<ControllerResolver>> controllerResolvers,
                      std::shared_ptr<ControllerResolver> notFoundResolver =
                          std::make_shared<Resolver::NotFoundControllerResolver>());

        Poco::Net::HTTPRequestHandler* createRequestHandler(const Poco::Net::HTTPServerRequest& request) override;

    private:
        std::vector<std::shared_ptr<ControllerResolver>> _controllerResolvers;
        const std::vector<std::shared_ptr<ControllerResolver>>& getControllerResolvers() const
        {
            return _controllerResolvers;
        }

        std::shared_ptr<ControllerResolver> _notFoundControllerResolver;
        const std::shared_ptr<ControllerResolver>& getNotFoundControllerResolver() const
        {
            return _notFoundControllerResolver;
        }
    };
} // namespace Rest
