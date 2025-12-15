#pragma once

#include <Poco/JSON/Object.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/URI.h>
#include <ctre.hpp>

#include "Rest/Controller/AuthController.h"
#include "Rest/Controller/NotFoundController.h"
#include "Util/Tuple.h"

namespace Rest
{
    template <ControllerResolverC NotfoundControllerResolverT,
              ControllerResolverC... ControllerResolverTs>
    RequestRouter<NotfoundControllerResolverT, ControllerResolverTs...>::RequestRouter(
        const std::tuple<ControllerResolverTs...>& controllerResolvers,
        NotfoundControllerResolverT&& notFoundResolver) :
        _controllerResolvers(controllerResolvers), _notFoundControllerResolver(notFoundResolver)
    {
    }

    template <ControllerResolverC NotfoundControllerResolverT,
              ControllerResolverC... ControllerResolverTs>
    Poco::Net::HTTPRequestHandler* RequestRouter<
        NotfoundControllerResolverT,
        ControllerResolverTs...>::createRequestHandler(const Poco::Net::HTTPServerRequest& request)
    {
        using namespace Rest;

        const auto uri = Poco::URI(request.getURI());

        std::optional<Poco::Net::HTTPRequestHandler*> handler = std::nullopt;
        Utils::TupleForEachWithContinue(getControllerResolvers(),
            [&](auto&& resolver) -> bool
            {
                if (resolver.match(uri))
                {
                    handler = resolver.make();
                    return false;
                }
                return true;
            });

        if (handler)
        {
            return *handler;
        }
        else
        {
            return new Controller::NotFoundController();
        }
    }
} // namespace Rest
