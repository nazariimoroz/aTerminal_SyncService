#include "Rest/RequestRouter.h"
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/JSON/Object.h>

#include "Rest/Controller/NotFoundController.h"
#include "Rest/Controller/UserController.h"

Poco::Net::HTTPRequestHandler* Api::RequestRouter::createRequestHandler(const Poco::Net::HTTPServerRequest& request)
{
    if (request.getURI() == "/api/v0/user")
    {
        return new Api::UserController();
    }

    return new Api::NotFoundController();
}
