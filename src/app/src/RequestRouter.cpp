#include "Rest/RequestRouter.h"
#include <Poco/JSON/Object.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/URI.h>
#include <ctre.hpp>

#include "Rest/Controller/AuthController.h"
#include "Rest/Controller/NotFoundController.h"

Rest::RequestRouter::RequestRouter(std::vector<std::shared_ptr<ControllerResolver>> controllerResolvers,
                                   std::shared_ptr<ControllerResolver> notFoundResolver) :
    _controllerResolvers(std::move(controllerResolvers)), _notFoundControllerResolver(std::move(notFoundResolver))
{
}

Poco::Net::HTTPRequestHandler* Rest::RequestRouter::createRequestHandler(const Poco::Net::HTTPServerRequest& request)
{
    using namespace Rest;

    const auto uri = Poco::URI(request.getURI());
    for (const auto& resolver : getControllerResolvers())
    {
        if (resolver->match(uri))
        {
            return resolver->make();
        }
    }

    return new Controller::NotFoundController();
}
