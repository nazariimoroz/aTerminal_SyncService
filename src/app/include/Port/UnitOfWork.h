#pragma once
#include <memory>

namespace Port
{
    class IUnitOfWork
    {
    public:
        virtual ~IUnitOfWork() = default;

        virtual void commit() = 0;
        virtual void rollback() = 0;
    };
}

