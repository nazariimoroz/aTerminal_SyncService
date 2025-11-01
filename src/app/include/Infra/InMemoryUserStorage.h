#pragma once
#include <Poco/Exception.h>
#include <shared_mutex>
#include <unordered_map>

#include "Port/User/UserUpdatableStorage.h"
#include "Util/BusinessException.h"

namespace Infra
{
    class InMemoryUserStorage final : public Port::User::IUserUpdatableStorage
    {
    public:
        ~InMemoryUserStorage() override;

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
