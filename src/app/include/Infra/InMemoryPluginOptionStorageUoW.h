#pragma once
#include <mutex>

#include "Port/UnitOfWork.h"

namespace Infra
{
    class InMemoryPluginOptionStorageUoW
    {
    public:
        InMemoryPluginOptionStorageUoW();

        ~InMemoryPluginOptionStorageUoW();

        void commit();

        void rollback();

        bool isActive() const;

        bool isCommitted() const;

    private:
        mutable std::mutex _mutex;
        bool _active;
        bool _committed;
    };

    static_assert(Port::UnitOfWorkC<InMemoryPluginOptionStorageUoW>);
} // namespace Infra
