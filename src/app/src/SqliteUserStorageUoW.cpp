#include "Infra/SqliteUserStorageUoW.h"

namespace Infra
{
    SqliteUserStorageUoW::SqliteUserStorageUoW(SQLite::Database& db, std::recursive_mutex& mutex)
        : _db(&db)
        , _lock(mutex)
        , _tx(std::make_unique<SQLite::Transaction>(db))
        , _active(true)
        , _committed(false)
    {
    }

    SqliteUserStorageUoW::~SqliteUserStorageUoW()
    {
        std::lock_guard<std::mutex> g(_mutex);
        if (!_active)
            return;

        _tx.reset();
        _active = false;
        _committed = false;
    }

    void SqliteUserStorageUoW::commit()
    {
        std::lock_guard<std::mutex> g(_mutex);
        if (!_active || !_tx)
            return;

        _tx->commit();
        _tx.reset();

        _committed = true;
        _active = false;

        if (_lock.owns_lock())
            _lock.unlock();
    }

    void SqliteUserStorageUoW::rollback()
    {
        std::lock_guard<std::mutex> g(_mutex);
        if (!_active || !_tx)
            return;

        _tx->rollback();
        _tx.reset();

        _committed = false;
        _active = false;

        if (_lock.owns_lock())
            _lock.unlock();
    }

    bool SqliteUserStorageUoW::isActive() const
    {
        std::lock_guard<std::mutex> g(_mutex);
        return _active;
    }

    bool SqliteUserStorageUoW::isCommitted() const
    {
        std::lock_guard<std::mutex> g(_mutex);
        return _committed;
    }
}
