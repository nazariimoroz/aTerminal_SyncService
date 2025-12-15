#pragma once

#include <concepts>
#include <optional>
#include <string>

#include "Domain/PluginOptions.h"

namespace Port::Plugin
{
    template <class T>
    concept PluginOptionsStorageC = requires(T t, int id, int userId, const std::string& pluginName) {
        { t.findById(id) } -> std::same_as<std::optional<Domain::PluginOptions>>;
        { t.findByUserIdAndName(userId, pluginName) } -> std::same_as<std::optional<Domain::PluginOptions>>;
    };
} // namespace Port::Plugin
