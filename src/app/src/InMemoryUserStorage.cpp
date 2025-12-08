#include "Infra/InMemoryUserStorage.h"

#include <stdexcept>

#include "Infra/InMemoryUserStorageUoW.h"
#include "Infra/InMemoryUserStorage.h"

using namespace Infra;

std::optional<Domain::User> InMemoryUserStorage::findById(const int& id)
{
    std::shared_lock lock(_mutex);

    auto it = _byId.find(id);
    if (it == _byId.end())
        return std::nullopt;
    return it->second;
}

std::optional<Domain::User> InMemoryUserStorage::findByEmail(const std::string& email)
{
    std::shared_lock lock(_mutex);

    auto it = _byEmail.find(email);
    if (it == _byEmail.end())
        return std::nullopt;
    return it->second;
}

bool InMemoryUserStorage::existsByEmail(const std::string& email)
{
    std::shared_lock lock(_mutex);
    return _byEmail.contains(email);
}

std::expected<void, Port::User::EmailAlreadyRegisteredError> InMemoryUserStorage::add(Domain::User& user)
{
    std::unique_lock lock(_mutex);

    if (_byEmail.contains(user.getEmail()))
    {
        return std::unexpected(Port::User::EmailAlreadyRegisteredError("User with such email is already registered"));
    }

    user.setId(generateId());

    _byId[user.getId()] = user;
    _byEmail[user.getEmail()] = user;
    return {};
}

void InMemoryUserStorage::update(const Domain::User& user)
{
    std::unique_lock lock(_mutex);

    _byId[user.getId()] = user;
    _byEmail[user.getEmail()] = user;
}

InMemoryUserStorageUoW InMemoryUserStorage::beginWork()
{
    return InMemoryUserStorageUoW();
}

int InMemoryUserStorage::generateId()
{
    _lastId += 1;
    return _lastId;
}
