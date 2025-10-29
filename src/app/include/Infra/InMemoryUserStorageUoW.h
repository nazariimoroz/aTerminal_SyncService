#pragma once
#include <mutex>

#include "Port/UnitOfWork.h"

namespace Infra
{
    class InMemoryUserStorageUoW final : public Port::IUnitOfWork
    {
    public:
        InMemoryUserStorageUoW()
            : _active(true)
            , _committed(false)
        {}

        ~InMemoryUserStorageUoW() override
        {
            std::lock_guard lock(_mutex);
            _active = false;
        }

        void commit() override
        {
            std::lock_guard lock(_mutex);
            if (!_active) return;
            _committed = true;
            _active    = false;
        }

        void rollback() override
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
}
