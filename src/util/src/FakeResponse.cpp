#include "Util/Test/FakeResponse.h"

Util::FakeResponse::FakeResponse() = default;
Util::FakeResponse::~FakeResponse() = default;

void Util::FakeResponse::sendContinue()
{
    _sentContinue = true;
}

std::ostream& Util::FakeResponse::send()
{
    return _out;
}

void Util::FakeResponse::sendFile(const std::string& path, const std::string& mediaType)
{
    setContentType(mediaType);

    std::ifstream ifs(path, std::ios::binary);
    if (ifs.good())
    {
        _out << ifs.rdbuf();
    }
}

void Util::FakeResponse::sendBuffer(const void* pBuffer, std::size_t length)
{
    const char* bytes = static_cast<const char*>(pBuffer);
    _out.write(bytes, static_cast<std::streamsize>(length));
}

void Util::FakeResponse::redirect(const std::string& uri, HTTPStatus status)
{
    setStatusAndReason(status);
    set("Location", uri);
    _redirectLocation = uri;
}

std::string Util::FakeResponse::body() const
{
    return _out.str();
}

bool Util::FakeResponse::sentContinue() const
{
    return _sentContinue;
}

const std::string& Util::FakeResponse::redirectLocation() const
{
    return _redirectLocation;
}

void Util::FakeResponse::requireAuthentication(const std::string& realm)
{
}

bool Util::FakeResponse::sent() const
{
    return false;
}
