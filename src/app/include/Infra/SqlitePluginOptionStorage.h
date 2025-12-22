#pragma once

#include <optional>
#include <string>
#include <unordered_map>
#include <mutex>

#include <SQLiteCpp/SQLiteCpp.h>

#include "Domain/PluginOptions.h"
#include "Infra/SqliteStorageUoW.h"
#include "Port/Plugin/PluginOptionsStorage.h"
#include "Port/Plugin/PluginOptionsUpdateableStorage.h"

namespace Infra
{
    class SqlitePluginOptionStorage
    {
    public:
        explicit SqlitePluginOptionStorage(SQLite::Database& db);

        std::optional<Domain::PluginOptions> findById(int id);
        std::optional<Domain::PluginOptions> findByUserIdAndName(int userId, const std::string& name);

        void update(const Domain::PluginOptions& po);

        SqliteStorageUoW beginWork();

    private:
        void ensureSchema();

        std::optional<Domain::PluginOptions> loadOneOption(SQLite::Statement& stmt);
        std::unordered_map<std::string, std::string> loadFields(int optionId);

        int upsertAndGetId(const Domain::PluginOptions& po);

    private:
        SQLite::Database* _db = nullptr;
        std::recursive_mutex _mutex;
    };

    static_assert(Port::Plugin::PluginOptionsStorageC<SqlitePluginOptionStorage>);
    static_assert(Port::Plugin::PluginOptionsUpdateableStorageC<SqlitePluginOptionStorage>);
}
