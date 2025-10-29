#pragma once

#include <Poco/Net/HTTPRequestHandler.h>

namespace Api
{
    class NotFoundController final : public Poco::Net::HTTPRequestHandler
    {
    public:
        void handleRequest(Poco::Net::HTTPServerRequest& request,
                           Poco::Net::HTTPServerResponse& response) override;
    };
}
