#include "Rest/RequestRouter.h"
#include <Poco/JSON/Object.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <ctre.hpp>

#include "Rest/Controller/AuthController.h"
#include "Rest/Controller/NotFoundController.h"

Rest::RequestRouter::RequestRouter(std::shared_ptr<Service::MessageBus> messageBus, Poco::Logger& logger) :
    _messageBus(std::move(messageBus)), _logger(logger)
{
    /* clang-format off */
    _routes =
    {
        {
            [&](const std::string& uri) -> bool
            {
                return ctre::match<R"(^/api/v0/auth/(?:register|login|refresh|logout)/?$)">(uri);
            },
            [&] -> Poco::Net::HTTPRequestHandler*
            {
                return new Controller::AuthController(getMessageBus(), getLogger());
            }
        },
    };
    /* clang-format on */
}

Poco::Net::HTTPRequestHandler* Rest::RequestRouter::createRequestHandler(const Poco::Net::HTTPServerRequest& request)
{
    using namespace Rest;

    const auto uri = request.getURI();
    for (const auto [validator, maker] : getRoutes())
    {
        if (validator(uri))
        {
            return maker();
        }
    }

    return new Controller::NotFoundController();
}
