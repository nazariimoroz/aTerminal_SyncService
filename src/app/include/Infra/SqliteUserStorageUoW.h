#pragma once

#include <mutex>
#include <memory>

#include <SQLiteCpp/SQLiteCpp.h>

#include "Port/UnitOfWork.h"

namespace Infra
{
    class SqliteUserStorageUoW
    {
    public:
        SqliteUserStorageUoW(SQLite::Database& db, std::recursive_mutex& mutex);
        ~SqliteUserStorageUoW();

        SqliteUserStorageUoW(const SqliteUserStorageUoW&) = delete;
        SqliteUserStorageUoW& operator=(const SqliteUserStorageUoW&) = delete;
        SqliteUserStorageUoW(SqliteUserStorageUoW&&) = delete;
        SqliteUserStorageUoW& operator=(SqliteUserStorageUoW&&) = delete;

        void commit();
        void rollback();

        bool isActive() const;
        bool isCommitted() const;

    private:
        SQLite::Database* _db = nullptr;
        std::unique_lock<std::recursive_mutex> _lock;
        std::unique_ptr<SQLite::Transaction> _tx;

        mutable std::mutex _mutex;
        bool _active = false;
        bool _committed = false;
    };

    static_assert(Port::UnitOfWorkC<SqliteUserStorageUoW>);
}
