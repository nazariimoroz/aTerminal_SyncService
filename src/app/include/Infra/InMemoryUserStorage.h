#pragma once
#include <Poco/Exception.h>
#include <shared_mutex>
#include <unordered_map>

#include "InMemoryUserStorageUoW.h"
#include "Port/User/UserUpdatableStorage.h"

namespace Infra
{
    class InMemoryUserStorage
    {
    public:
        std::optional<Domain::User> findById(const int& id);
        std::optional<Domain::User> findByEmail(const std::string& email);
        bool existsByEmail(const std::string& email);
        std::expected<void, Port::User::EmailAlreadyRegisteredError> add(Domain::User& user);
        void update(const Domain::User& user);

        InMemoryUserStorageUoW beginWork();

    protected:
        int generateId();

    private:
        std::unordered_map<int, Domain::User> _byId;
        std::unordered_map<std::string, Domain::User> _byEmail;

        int _lastId = 0;
        mutable std::shared_mutex _mutex;
    };

    static_assert(Port::User::UserStorageC<InMemoryUserStorage>);
    static_assert(Port::User::UserUpdatableStorageC<InMemoryUserStorage>);

} // namespace Infra
