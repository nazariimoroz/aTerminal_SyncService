#include "Infra/InMemoryUserStorage.h"

#include <stdexcept>

#include "Infra/InMemoryUserStorageUoW.h"

namespace Infra
{
    std::optional<Domain::User> InMemoryUserStorage::findById(const uuids::uuid& id)
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

    void InMemoryUserStorage::add(const Domain::User& user)
    {
        std::unique_lock lock(_mutex);

        if (_byEmail.contains(user.getEmail()))
        {
            throw std::runtime_error("email already registered");
        }

        _byId[user.getId()]       = user;
        _byEmail[user.getEmail()] = user;
    }

    void InMemoryUserStorage::update(const Domain::User& user)
    {
        std::unique_lock lock(_mutex);

        _byId[user.getId()] = user;
        _byEmail[user.getEmail()] = user;
    }

    std::unique_ptr<Port::IUnitOfWork> InMemoryUserStorage::beginWork()
    {
        return std::make_unique<InMemoryUserStorageUoW>();
    }
} // namespace Infra
