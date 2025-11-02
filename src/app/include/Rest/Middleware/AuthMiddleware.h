#pragma once
#include <Poco/Exception.h>
#include <memory>

#include "Util/BusinessException.h"

namespace Service
{
    class MessageBus;
}
namespace Poco::Net
{
    class HTTPServerResponse;
    class HTTPServerRequest;
}

namespace Rest::Middleware
{
    POCO_DECLARE_EXCEPTION(, AuthFailedException, Util::BusinessException)

    struct AuthMiddlewareResult
    {
        int id;
    };

    class AuthMiddleware
    {
    public:
        explicit AuthMiddleware(std::shared_ptr<Service::MessageBus> messageBus);

        AuthMiddlewareResult execute(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response) const;

    protected:
        std::shared_ptr<Service::MessageBus> _messageBus;
        const std::shared_ptr<Service::MessageBus>& getMessageBus() const { return _messageBus; }
    };
}

