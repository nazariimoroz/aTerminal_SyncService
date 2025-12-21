#include "Infra/SqliteUserStorage.h"

#include <cstring>
#include <sqlite3.h>

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

        static int toInt(const Domain::VerificationMethod m)
        {
            return static_cast<int>(static_cast<std::uint8_t>(m));
        }

        static Domain::VerificationMethod fromInt(const int v)
        {
            return static_cast<Domain::VerificationMethod>(static_cast<std::uint8_t>(v));
        }

        static std::vector<char> blobToVec(const SQLite::Column& col)
        {
            const int bytes = col.getBytes();
            if (bytes <= 0)
                return {};

            const void* p = col.getBlob();
            std::vector<char> out(static_cast<size_t>(bytes));
            if (p)
                std::memcpy(out.data(), p, static_cast<size_t>(bytes));
            return out;
        }
    }

    SqliteUserStorage::SqliteUserStorage(SQLite::Database& db)
        : _db(&db)
    {
        std::lock_guard<std::recursive_mutex> lock(_mutex);

        _db->exec("PRAGMA foreign_keys = ON;");
        _db->exec("PRAGMA busy_timeout = 5000;");

        ensureSchema();
    }

    void SqliteUserStorage::ensureSchema()
    {
        _db->exec(
            "CREATE TABLE IF NOT EXISTS users ("
            "  id INTEGER PRIMARY KEY,"
            "  email TEXT NOT NULL UNIQUE,"
            "  password BLOB NOT NULL,"
            "  verification_method INTEGER NOT NULL"
            ");");

        _db->exec("CREATE INDEX IF NOT EXISTS idx_users_email ON users(email);");
    }

    std::optional<Domain::User> SqliteUserStorage::findById(const int& id)
    {
        std::lock_guard<std::recursive_mutex> lock(_mutex);

        SQLite::Statement stmt(
            *_db,
            "SELECT id, email, password, verification_method "
            "FROM users "
            "WHERE id = ?1 "
            "LIMIT 1;");

        stmt.bind(1, id);

        if (!stmt.executeStep())
            return std::nullopt;

        Domain::User u;
        u.setId(stmt.getColumn(0).getInt());
        u.setEmail(stmt.getColumn(1).getString());
        u.setPassword(blobToVec(stmt.getColumn(2)));
        u.setVerificationMethod(fromInt(stmt.getColumn(3).getInt()));
        return u;
    }

    std::optional<Domain::User> SqliteUserStorage::findByEmail(const std::string& email)
    {
        std::lock_guard<std::recursive_mutex> lock(_mutex);

        SQLite::Statement stmt(
            *_db,
            "SELECT id, email, password, verification_method "
            "FROM users "
            "WHERE email = ?1 "
            "LIMIT 1;");

        stmt.bind(1, email);

        if (!stmt.executeStep())
            return std::nullopt;

        Domain::User u;
        u.setId(stmt.getColumn(0).getInt());
        u.setEmail(stmt.getColumn(1).getString());
        u.setPassword(blobToVec(stmt.getColumn(2)));
        u.setVerificationMethod(fromInt(stmt.getColumn(3).getInt()));
        return u;
    }

    bool SqliteUserStorage::existsByEmail(const std::string& email)
    {
        std::lock_guard<std::recursive_mutex> lock(_mutex);

        SQLite::Statement stmt(
            *_db,
            "SELECT 1 "
            "FROM users "
            "WHERE email = ?1 "
            "LIMIT 1;");

        stmt.bind(1, email);
        return stmt.executeStep();
    }

    std::expected<void, Port::User::EmailAlreadyRegisteredError> SqliteUserStorage::add(Domain::User& user)
    {
        std::lock_guard<std::recursive_mutex> lock(_mutex);

        SavepointGuard sp(*_db, "users_add");

        const auto& pw = user.getPassword();
        const void* pwPtr = pw.empty() ? "" : static_cast<const void*>(pw.data());
        const int pwSize = static_cast<int>(pw.size());

        try
        {
            SQLite::Statement stmt(
                *_db,
                "INSERT INTO users(email, password, verification_method) "
                "VALUES (?1, ?2, ?3);");

            stmt.bind(1, user.getEmail());
            stmt.bind(2, pwPtr, pwSize);
            stmt.bind(3, toInt(user.getVerificationMethod()));
            stmt.exec();
        }
        catch (const SQLite::Exception& e)
        {
            if (e.getErrorCode() == SQLITE_CONSTRAINT || e.getErrorCode() == SQLITE_CONSTRAINT_UNIQUE)
            {
                return std::unexpected(
                    Port::User::EmailAlreadyRegisteredError("User with such email is already registered"));
            }
            throw;
        }

        user.setId(static_cast<int>(_db->getLastInsertRowid()));

        sp.release();
        return {};
    }

    void SqliteUserStorage::update(const Domain::User& user)
    {
        std::lock_guard<std::recursive_mutex> lock(_mutex);

        SavepointGuard sp(*_db, "users_update");

        const auto& pw = user.getPassword();
        const void* pwPtr = pw.empty() ? "" : static_cast<const void*>(pw.data());
        const int pwSize = static_cast<int>(pw.size());

        SQLite::Statement stmt(
            *_db,
            "INSERT INTO users(id, email, password, verification_method) "
            "VALUES (?1, ?2, ?3, ?4) "
            "ON CONFLICT(id) DO UPDATE SET "
            "  email = excluded.email, "
            "  password = excluded.password, "
            "  verification_method = excluded.verification_method;");

        stmt.bind(1, user.getId());
        stmt.bind(2, user.getEmail());
        stmt.bind(3, pwPtr, pwSize);
        stmt.bind(4, toInt(user.getVerificationMethod()));
        stmt.exec();

        sp.release();
    }

    SqliteUserStorageUoW SqliteUserStorage::beginWork()
    {
        return SqliteUserStorageUoW(*_db, _mutex);
    }
}

