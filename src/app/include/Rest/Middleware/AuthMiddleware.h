#pragma once
#include <Poco/Exception.h>
#include <expected>
#include <memory>

#include "Util/Errors.h"

namespace Service
{
    class MessageBus;
}
namespace Poco::Net
{
    class HTTPServerResponse;
    class HTTPServerRequest;
} // namespace Poco::Net

namespace Rest::Middleware
{
    struct AuthFailedError : Error::StrError
    {
        using StrError::StrError;
    };

    struct AuthMiddlewareResult
    {
        int id;
    };

    class AuthMiddleware
    {
    public:
        explicit AuthMiddleware(Service::MessageBus& messageBus);

        std::expected<AuthMiddlewareResult, AuthFailedError> execute(
            Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response) const;

    protected:
        Service::MessageBus& _messageBus;
        Service::MessageBus& getMessageBus() const
        {
            return _messageBus;
        }
    };
} // namespace Rest::Middleware
