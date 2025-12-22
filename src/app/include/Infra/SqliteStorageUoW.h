#pragma once

#include <mutex>
#include <memory>

#include <SQLiteCpp/SQLiteCpp.h>

#include "Port/UnitOfWork.h"

namespace Infra
{
    class SqliteStorageUoW
    {
    public:
        SqliteStorageUoW(SQLite::Database& db, std::recursive_mutex& mutex);
        ~SqliteStorageUoW();

        SqliteStorageUoW(const SqliteStorageUoW&) = delete;
        SqliteStorageUoW& operator=(const SqliteStorageUoW&) = delete;
        SqliteStorageUoW(SqliteStorageUoW&&) = delete;
        SqliteStorageUoW& operator=(SqliteStorageUoW&&) = delete;

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

    static_assert(Port::UnitOfWorkC<SqliteStorageUoW>);
}
