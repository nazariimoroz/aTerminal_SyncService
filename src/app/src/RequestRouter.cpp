#include "Rest/RequestRouter.h"
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/JSON/Object.h>

#include "Rest/Controller/NotFoundController.h"
#include "Rest/Controller/UserController.h"

Poco::Net::HTTPRequestHandler* Rest::RequestRouter::createRequestHandler(const Poco::Net::HTTPServerRequest& request)
{
    using namespace Rest;

    const auto u = request.getURI();
    if (u == "/api/v0/user") return new Controller::UserController();

    return new Controller::NotFoundController();
}
