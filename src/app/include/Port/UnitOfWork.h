#pragma once
#include <memory>

namespace Port
{
    template<class T>
    concept UnitOfWorkC = requires(T t)
    {
        { t.commit() } -> std::same_as<void>;
        { t.rollback() } -> std::same_as<void>;
    };
}

