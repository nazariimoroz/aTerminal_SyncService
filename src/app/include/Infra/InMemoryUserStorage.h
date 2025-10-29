#pragma once
#include <unordered_map>
#include <shared_mutex>

#include "Port/User/UserUpdatableStorage.h"

namespace Infra
{
    class InMemoryUserStorage : public Port::User::IUserUpdatableStorage
    {
    public:
        std::optional<Domain::User> findById(const int& id) override;
        std::optional<Domain::User> findByEmail(const std::string& email) override;
        bool existsByEmail(const std::string& email) override;
        void add(Domain::User& user) override;
        void update(const Domain::User& user) override;

        std::unique_ptr<Port::IUnitOfWork> beginWork() override;

    protected:
        int generateId();

    private:
        std::unordered_map<int, Domain::User> _byId;
        std::unordered_map<std::string, Domain::User> _byEmail;

        int _lastId = 0;
        mutable std::shared_mutex _mutex;
    };
} // namespace Infra
