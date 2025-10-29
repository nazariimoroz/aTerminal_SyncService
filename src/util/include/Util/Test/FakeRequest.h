#pragma once

#include <sstream>
#include <utility>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include "FakeResponse.h"

namespace Util
{
    class FakeRequest : public Poco::Net::HTTPServerRequest
    {
    public:
        FakeRequest(const std::string& method,
                    const std::string& uri,
                    const std::string& body,
                    FakeResponse& resp,
                    const std::string& clientHost = "127.0.0.1",
                    unsigned short clientPort     = 12345,
                    const std::string& serverHost = "127.0.0.1",
                    unsigned short serverPort     = 8080);
        ~FakeRequest() override  = default;

        std::istream& stream() override;
        const Poco::Net::SocketAddress& clientAddress() const override;
        const Poco::Net::SocketAddress& serverAddress() const override;
        const Poco::Net::HTTPServerParams& serverParams() const override;
        Poco::Net::HTTPServerResponse& response() const override;
        bool secure() const override;

    private:
        std::istringstream          _bodyStream;
        FakeResponse&               _response;
        Poco::Net::SocketAddress    _clientAddr;
        Poco::Net::SocketAddress    _serverAddr;
    };
}
