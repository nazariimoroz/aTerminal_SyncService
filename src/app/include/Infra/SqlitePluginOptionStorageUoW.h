// Infra/SqlitePluginOptionStorageUoW.h
#pragma once

#include <mutex>
#include <memory>

#include <SQLiteCpp/SQLiteCpp.h>

#include "Port/UnitOfWork.h"

namespace Infra
{
    class SqlitePluginOptionStorageUoW
    {
    public:
        SqlitePluginOptionStorageUoW(SQLite::Database& db, std::recursive_mutex& mutex);
        ~SqlitePluginOptionStorageUoW();

        void commit();
        void rollback();

        bool isActive() const { return _active; }
        bool isCommitted() const { return _committed; }

        SqlitePluginOptionStorageUoW(const SqlitePluginOptionStorageUoW&) = delete;
        SqlitePluginOptionStorageUoW& operator=(const SqlitePluginOptionStorageUoW&) = delete;

        SqlitePluginOptionStorageUoW(SqlitePluginOptionStorageUoW&&) noexcept = default;
        SqlitePluginOptionStorageUoW& operator=(SqlitePluginOptionStorageUoW&&) noexcept = default;

    private:
        SQLite::Database* _db = nullptr;
        std::unique_lock<std::recursive_mutex> _lock;

        std::unique_ptr<SQLite::Transaction> _tx;

        bool _active = false;
        bool _committed = false;
    };

    static_assert(Port::UnitOfWorkC<SqlitePluginOptionStorageUoW>);
}
