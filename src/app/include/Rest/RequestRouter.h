#pragma once

#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <functional>

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
        RequestRouter(std::shared_ptr<Service::MessageBus> messageBus, Poco::Logger& logger);

        Poco::Net::HTTPRequestHandler* createRequestHandler(const Poco::Net::HTTPServerRequest& request) override;

    protected:
        std::shared_ptr<Service::MessageBus> _messageBus;
        const std::shared_ptr<Service::MessageBus>& getMessageBus() const
        {
            return _messageBus;
        }

        Poco::Logger& _logger;
        Poco::Logger& getLogger() const
        {
            return _logger;
        }

    private:
        std::vector<Route> _routes;
        const std::vector<Route>& getRoutes() const
        {
            return _routes;
        }
    };
} // namespace Rest
