#include "Infra/InMemoryPluginOptionStorageUoW.h"

namespace Infra
{
    InMemoryPluginOptionStorageUoW::InMemoryPluginOptionStorageUoW() :
        _active(true), _committed(false)
    {
    }
    InMemoryPluginOptionStorageUoW::~InMemoryPluginOptionStorageUoW()
    {
        std::lock_guard lock(_mutex);
        _active = false;
    }
    void InMemoryPluginOptionStorageUoW::commit()
    {
        std::lock_guard lock(_mutex);
        if (!_active)
            return;
        _committed = true;
        _active = false;
    }
    void InMemoryPluginOptionStorageUoW::rollback()
    {
        std::lock_guard lock(_mutex);
        if (!_active)
            return;
        _committed = false;
        _active = false;
    }
    bool InMemoryPluginOptionStorageUoW::isActive() const
    {
        std::lock_guard lock(_mutex);
        return _active;
    }
    bool InMemoryPluginOptionStorageUoW::isCommitted() const
    {
        std::lock_guard lock(_mutex);
        return _committed;
    }
} // namespace Infra
