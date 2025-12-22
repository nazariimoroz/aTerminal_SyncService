#pragma once

#include <Poco/Exception.h>
#include <expected>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

#include <SQLiteCpp/SQLiteCpp.h>

#include "Infra/SqliteStorageUoW.h"
#include "Port/User/UserUpdatableStorage.h"

namespace Infra
{
    class SqliteUserStorage
    {
    public:
        explicit SqliteUserStorage(SQLite::Database& db);

        std::optional<Domain::User> findById(const int& id);
        std::optional<Domain::User> findByEmail(const std::string& email);
        bool existsByEmail(const std::string& email);
        std::expected<void, Port::User::EmailAlreadyRegisteredError> add(Domain::User& user);
        void update(const Domain::User& user);

        SqliteStorageUoW beginWork();

    private:
        void ensureSchema();

    private:
        SQLite::Database* _db = nullptr;
        std::recursive_mutex _mutex;
    };

    static_assert(Port::User::UserStorageC<SqliteUserStorage>);
    static_assert(Port::User::UserUpdatableStorageC<SqliteUserStorage>);
}
