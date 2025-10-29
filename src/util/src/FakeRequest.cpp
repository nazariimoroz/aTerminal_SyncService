#include "Util/Test/FakeRequest.h"
#include <Poco/Net/HTTPServerParams.h>

Util::FakeRequest::FakeRequest(const std::string& method, const std::string& uri, const std::string& body,
    FakeResponse& resp, const std::string& clientHost, unsigned short clientPort, const std::string& serverHost,
    unsigned short serverPort): _bodyStream(body)
                                , _response(resp)
                                , _clientAddr(clientHost, clientPort)
                                , _serverAddr(serverHost, serverPort)
{
    setMethod(method);
    setURI(uri);
    setVersion("HTTP/1.1");
    setExpectContinue(false);

    setContentLength(static_cast<std::streamsize>(body.size()));
}

std::istream& Util::FakeRequest::stream()
{
    return _bodyStream;
}

const Poco::Net::SocketAddress& Util::FakeRequest::clientAddress() const
{
    return _clientAddr;
}

const Poco::Net::SocketAddress& Util::FakeRequest::serverAddress() const
{
    return _serverAddr;
}

const Poco::Net::HTTPServerParams& Util::FakeRequest::serverParams() const
{
    static auto FakeParams = new Poco::Net::HTTPServerParams;
    return *FakeParams;
}

Poco::Net::HTTPServerResponse& Util::FakeRequest::response() const
{
    return _response;
}

bool Util::FakeRequest::secure() const
{
    return false;
}
