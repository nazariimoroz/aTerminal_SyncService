// InMemoryPluginOptionStorage.h
#pragma once

#include <optional>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <utility>

#include "Domain/PluginOptions.h"
#include "Infra/InMemoryPluginOptionStorageUoW.h"
#include "Port/Plugin/PluginOptionsStorage.h"

namespace Infra
{
    class InMemoryPluginOptionStorage
    {
    public:
        InMemoryPluginOptionStorage();

        std::optional<Domain::PluginOptions> findById(const int id);

        std::optional<Domain::PluginOptions> findByUserIdAndName(const int userId, const std::string& name);

        void update(const Domain::PluginOptions& po);

        InMemoryPluginOptionStorageUoW beginWork();

    private:
        static std::string makeUserNameKey(const int userId, const std::string& name);

    private:
        mutable std::shared_mutex _mutex;
        std::unordered_map<int, Domain::PluginOptions> _byId;
        std::unordered_map<std::string, Domain::PluginOptions> _byUserIdAndName;
    };

    static_assert(Port::Plugin::PluginOptionsStorageC<InMemoryPluginOptionStorage>);
} // namespace Infra
