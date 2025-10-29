#include "Rest/Controller/NotFoundController.h"
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/JSON/Object.h>

void Api::NotFoundController::handleRequest(Poco::Net::HTTPServerRequest& request,
                                            Poco::Net::HTTPServerResponse& response)
{
    response.setStatus(Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
    response.setContentType("application/json");

    Poco::JSON::Object::Ptr json = new Poco::JSON::Object;
    json->set("error", "Not Found");
    json->set("path", request.getURI());

    std::ostringstream oss;
    Poco::JSON::Stringifier::stringify(json, oss);

    std::ostream& out = response.send();
    out << oss.str();
}
