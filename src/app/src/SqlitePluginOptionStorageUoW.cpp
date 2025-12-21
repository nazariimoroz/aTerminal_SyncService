// Infra/SqlitePluginOptionStorageUoW.cpp
#include "Infra/SqlitePluginOptionStorageUoW.h"

namespace Infra
{
    SqlitePluginOptionStorageUoW::SqlitePluginOptionStorageUoW(SQLite::Database& db, std::recursive_mutex& mutex)
        : _db(&db)
        , _lock(mutex)
        , _tx(std::make_unique<SQLite::Transaction>(db))
        , _active(true)
        , _committed(false)
    {
    }

    SqlitePluginOptionStorageUoW::~SqlitePluginOptionStorageUoW()
    {
        if (!_active)
            return;

        _tx.reset();
        _active = false;
        _committed = false;
    }

    void SqlitePluginOptionStorageUoW::commit()
    {
        if (!_active || !_tx)
            return;

        _tx->commit();
        _tx.reset();

        _committed = true;
        _active = false;

        if (_lock.owns_lock())
            _lock.unlock();
    }

    void SqlitePluginOptionStorageUoW::rollback()
    {
        if (!_active || !_tx)
            return;

        _tx->rollback();
        _tx.reset();

        _committed = false;
        _active = false;

        if (_lock.owns_lock())
            _lock.unlock();
    }
}
