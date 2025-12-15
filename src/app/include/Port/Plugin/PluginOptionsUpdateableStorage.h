#pragma once

#include <concepts>
#include <expected>
#include <optional>
#include <string>

#include "Domain/PluginOptions.h"
#include "Port/Plugin/PluginOptionsStorage.h"
#include "Port/UnitOfWork.h"

namespace Port::Plugin
{
    template <class T>
    concept PluginOptionsUpdateableStorageC =
        PluginOptionsStorageC<T> && requires(T t, Domain::PluginOptions& poMut, const Domain::PluginOptions& poConst) {
            { t.update(poConst) } -> std::same_as<void>;
            { t.beginWork() } -> Port::UnitOfWorkC;
        };
} // namespace Port::Plugin
