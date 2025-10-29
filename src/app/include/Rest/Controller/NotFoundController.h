#pragma once

#include <Poco/Net/HTTPRequestHandler.h>

namespace Rest::Controller
{
    class NotFoundController final : public Poco::Net::HTTPRequestHandler
    {
    public:
        void handleRequest(Poco::Net::HTTPServerRequest& request,
                           Poco::Net::HTTPServerResponse& response) override;
    };
}
