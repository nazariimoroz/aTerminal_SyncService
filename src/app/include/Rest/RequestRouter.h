#pragma once

#include <Poco/Net/HTTPRequestHandlerFactory.h>

namespace Api
{
    class RequestRouter final : public Poco::Net::HTTPRequestHandlerFactory {
    public:
        RequestRouter() = default;

        Poco::Net::HTTPRequestHandler* createRequestHandler(
            const Poco::Net::HTTPServerRequest& request) override;
    };
}