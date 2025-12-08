#pragma once
#include <mutex>

#include "Port/UnitOfWork.h"

namespace Infra
{
    class InMemoryUserStorageUoW
    {
    public:
        InMemoryUserStorageUoW()
            : _active(true)
            , _committed(false)
        {}

        ~InMemoryUserStorageUoW()
        {
            std::lock_guard lock(_mutex);
            _active = false;
        }

        void commit()
        {
            std::lock_guard lock(_mutex);
            if (!_active) return;
            _committed = true;
            _active    = false;
        }

        void rollback()
        {
            std::lock_guard lock(_mutex);
            if (!_active) return;
            _committed = false;
            _active    = false;
        }

        bool isActive() const
        {
            std::lock_guard lock(_mutex);
            return _active;
        }

        bool isCommitted() const
        {
            std::lock_guard lock(_mutex);
            return _committed;
        }

    private:
        mutable std::mutex _mutex;
        bool _active;
        bool _committed;
    };

    static_assert(Port::UnitOfWorkC<InMemoryUserStorageUoW>);
}
