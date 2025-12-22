#include "Infra/SqlitePluginOptionStorage.h"

namespace Infra
{
    namespace
    {
        struct SavepointGuard
        {
            SQLite::Database& db;
            const char* name;
            bool released = false;

            SavepointGuard(SQLite::Database& d, const char* n) : db(d), name(n)
            {
                db.exec(std::string("SAVEPOINT ") + name + ";");
            }

            void release()
            {
                if (released) return;
                db.exec(std::string("RELEASE ") + name + ";");
                released = true;
            }

            ~SavepointGuard()
            {
                if (released) return;
                try
                {
                    db.exec(std::string("ROLLBACK TO ") + name + ";");
                    db.exec(std::string("RELEASE ") + name + ";");
                }
                catch (...)
                {
                }
            }
        };
    }

    SqlitePluginOptionStorage::SqlitePluginOptionStorage(SQLite::Database& db)
        : _db(&db)
    {
        std::lock_guard<std::recursive_mutex> lock(_mutex);

        _db->exec("PRAGMA foreign_keys = ON;");
        _db->exec("PRAGMA busy_timeout = 5000;");

        ensureSchema();
    }

    void SqlitePluginOptionStorage::ensureSchema()
    {
        _db->exec(
            "CREATE TABLE IF NOT EXISTS plugin_options ("
            "  id          INTEGER PRIMARY KEY,"
            "  user_id     INTEGER NOT NULL,"
            "  plugin_name TEXT    NOT NULL,"
            "  is_enabled  INTEGER NOT NULL,"
            "  is_favorite INTEGER NOT NULL,"
            "  UNIQUE(user_id, plugin_name)"
            ");");

        _db->exec(
            "CREATE TABLE IF NOT EXISTS plugin_option_fields ("
            "  option_id INTEGER NOT NULL,"
            "  key       TEXT    NOT NULL,"
            "  value     TEXT    NOT NULL,"
            "  PRIMARY KEY(option_id, key),"
            "  FOREIGN KEY(option_id) REFERENCES plugin_options(id) ON DELETE CASCADE"
            ");");

        _db->exec("CREATE INDEX IF NOT EXISTS idx_plugin_options_user_name ON plugin_options(user_id, plugin_name);");
        _db->exec("CREATE INDEX IF NOT EXISTS idx_plugin_option_fields_option_id ON plugin_option_fields(option_id);");
    }

    std::optional<Domain::PluginOptions> SqlitePluginOptionStorage::findById(const int id)
    {
        std::lock_guard<std::recursive_mutex> lock(_mutex);

        SQLite::Statement stmt(
            *_db,
            "SELECT id, user_id, plugin_name, is_enabled, is_favorite "
            "FROM plugin_options "
            "WHERE id = ?1 "
            "LIMIT 1;");

        stmt.bind(1, id);
        return loadOneOption(stmt);
    }

    std::optional<Domain::PluginOptions> SqlitePluginOptionStorage::findByUserIdAndName(
        const int userId, const std::string& name)
    {
        std::lock_guard<std::recursive_mutex> lock(_mutex);

        SQLite::Statement stmt(
            *_db,
            "SELECT id, user_id, plugin_name, is_enabled, is_favorite "
            "FROM plugin_options "
            "WHERE user_id = ?1 AND plugin_name = ?2 "
            "LIMIT 1;");

        stmt.bind(1, userId);
        stmt.bind(2, name);
        return loadOneOption(stmt);
    }

    std::optional<Domain::PluginOptions> SqlitePluginOptionStorage::loadOneOption(SQLite::Statement& stmt)
    {
        if (!stmt.executeStep())
            return std::nullopt;

        Domain::PluginOptions po;

        po.setId(stmt.getColumn(0).getInt());
        po.setUserId(stmt.getColumn(1).getInt());
        po.setPluginName(stmt.getColumn(2).getString());
        po.setEnabled(stmt.getColumn(3).getInt() != 0);
        po.setFavorite(stmt.getColumn(4).getInt() != 0);

        po.setFields(loadFields(po.getId()));
        return po;
    }

    std::unordered_map<std::string, std::string> SqlitePluginOptionStorage::loadFields(const int optionId)
    {
        std::unordered_map<std::string, std::string> out;

        SQLite::Statement stmt(
            *_db,
            "SELECT key, value "
            "FROM plugin_option_fields "
            "WHERE option_id = ?1;");

        stmt.bind(1, optionId);

        while (stmt.executeStep())
        {
            out.emplace(stmt.getColumn(0).getString(), stmt.getColumn(1).getString());
        }

        return out;
    }

    int SqlitePluginOptionStorage::upsertAndGetId(const Domain::PluginOptions& po)
    {
        if (po.getId() > 0)
        {
            SQLite::Statement stmt(
                *_db,
                "INSERT INTO plugin_options(id, user_id, plugin_name, is_enabled, is_favorite) "
                "VALUES (?1, ?2, ?3, ?4, ?5) "
                "ON CONFLICT(id) DO UPDATE SET "
                "  user_id     = excluded.user_id, "
                "  plugin_name = excluded.plugin_name, "
                "  is_enabled  = excluded.is_enabled, "
                "  is_favorite = excluded.is_favorite;");

            stmt.bind(1, po.getId());
            stmt.bind(2, po.getUserId());
            stmt.bind(3, po.getPluginName());
            stmt.bind(4, po.isEnabled() ? 1 : 0);
            stmt.bind(5, po.isFavorite() ? 1 : 0);

            stmt.exec();
            return po.getId();
        }

        {
            SQLite::Statement stmt(
                *_db,
                "INSERT INTO plugin_options(user_id, plugin_name, is_enabled, is_favorite) "
                "VALUES (?1, ?2, ?3, ?4) "
                "ON CONFLICT(user_id, plugin_name) DO UPDATE SET "
                "  is_enabled  = excluded.is_enabled, "
                "  is_favorite = excluded.is_favorite;");

            stmt.bind(1, po.getUserId());
            stmt.bind(2, po.getPluginName());
            stmt.bind(3, po.isEnabled() ? 1 : 0);
            stmt.bind(4, po.isFavorite() ? 1 : 0);

            stmt.exec();
        }

        SQLite::Statement q(
            *_db,
            "SELECT id FROM plugin_options WHERE user_id = ?1 AND plugin_name = ?2 LIMIT 1;");

        q.bind(1, po.getUserId());
        q.bind(2, po.getPluginName());

        if (!q.executeStep())
            return 0;

        return q.getColumn(0).getInt();
    }

    void SqlitePluginOptionStorage::update(const Domain::PluginOptions& po)
    {
        std::lock_guard<std::recursive_mutex> lock(_mutex);

        SavepointGuard sp(*_db, "plugin_options_update");

        const int optionId = upsertAndGetId(po);
        if (optionId <= 0)
            throw SQLite::Exception("Failed to upsert plugin_options", 0);

        {
            SQLite::Statement del(*_db, "DELETE FROM plugin_option_fields WHERE option_id = ?1;");
            del.bind(1, optionId);
            del.exec();
        }

        {
            SQLite::Statement ins(
                *_db,
                "INSERT INTO plugin_option_fields(option_id, key, value) "
                "VALUES (?1, ?2, ?3);");

            for (const auto& [k, v] : po.getFields())
            {
                ins.reset();
                ins.clearBindings();

                ins.bind(1, optionId);
                ins.bind(2, k);
                ins.bind(3, v);
                ins.exec();
            }
        }

        sp.release();
    }

    SqliteStorageUoW SqlitePluginOptionStorage::beginWork()
    {
        return { *_db, _mutex };
    }
}
