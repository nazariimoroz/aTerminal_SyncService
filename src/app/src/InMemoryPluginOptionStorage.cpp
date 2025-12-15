#include "Infra/InMemoryPluginOptionStorage.h"

namespace Infra
{
    InMemoryPluginOptionStorage::InMemoryPluginOptionStorage() = default;
    std::optional<Domain::PluginOptions> InMemoryPluginOptionStorage::findById(const int id)
    {
        std::shared_lock lock(_mutex);

        auto it = _byId.find(id);
        if (it == _byId.end())
            return std::nullopt;
        return it->second;
    }
    std::optional<Domain::PluginOptions> InMemoryPluginOptionStorage::findByUserIdAndName(
        const int userId, const std::string& name)
    {
        std::shared_lock lock(_mutex);

        const auto key = makeUserNameKey(userId, name);
        auto it = _byUserIdAndName.find(key);
        if (it == _byUserIdAndName.end())
            return std::nullopt;
        return it->second;
    }
    void InMemoryPluginOptionStorage::update(const Domain::PluginOptions& po)
    {
        std::unique_lock lock(_mutex);

        _byId[po.getId()] = po;
        _byUserIdAndName[makeUserNameKey(po.getUserId(), po.getPluginName())] = po;
    }
    InMemoryPluginOptionStorageUoW InMemoryPluginOptionStorage::beginWork()
    {
        return InMemoryPluginOptionStorageUoW();
    }
    std::string InMemoryPluginOptionStorage::makeUserNameKey(const int userId,
                                                             const std::string& name)
    {
        std::string key;
        key.reserve(32 + name.size());
        key.append(std::to_string(userId));
        key.push_back(':');
        key.append(name);
        return key;
    }
} // namespace Infra
