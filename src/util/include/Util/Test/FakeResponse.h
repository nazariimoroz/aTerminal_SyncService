#pragma once

#include <fstream>
#include <Poco/Net/HTTPServerResponse.h>
#include <sstream>

namespace Util
{
    class FakeResponse final : public Poco::Net::HTTPServerResponse
    {
    public:
        FakeResponse();
        ~FakeResponse() override;

        void sendContinue() override;

        std::ostream& send() override;

        void sendFile(const std::string& path,
                      const std::string& mediaType) override;

        void sendBuffer(const void* pBuffer,
                        std::size_t length) override;

        void redirect(const std::string& uri,
                      HTTPStatus status = HTTP_FOUND) override;

        std::string body() const;

        bool sentContinue() const;
        const std::string& redirectLocation() const;

        void requireAuthentication(const std::string& realm) override;

        bool sent() const override;

    private:
        std::ostringstream _out;
        bool _sentContinue = false;
        std::string _redirectLocation;
    };
}